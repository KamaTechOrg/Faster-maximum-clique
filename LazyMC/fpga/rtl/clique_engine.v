// Maximum-clique search engine for one subproblem (up to N vertices).
//
// Algorithm (identical to fpga/model/mc_model.cpp, cycle for cycle):
//   node(P, L): greedy colouring of P -> k colours, last coloured vertex v
//               if L + k <= best: pop
//               push continuation P\{v}, R[L] = v, L++
//               NP = P & N(v); NP == 0 ? (record L if better; pop) : node(NP, L)
//
// The "adjacency gate": every set operation is one N-bit wide AND on a row of
// the adjacency matrix read from block RAM in a single cycle:
//   colouring step   Q <= Q & ~N(v)          (1 vertex per 2 cycles)
//   branching step   NP = P & N(v)           (1 cycle)
//   next vertex      ffs(Q)                  (log2 N mux tree, same cycle)
//
// Memories (all on chip):
//   adj  : N x N bits   adjacency matrix (loaded by the host)
//   stk  : D x N bits   continuation set P of every stack level
//   rv   : D x LOGN     current clique R
//   brv  : D x LOGN     best clique found
module clique_engine #(
    parameter integer N    = 1024,
    parameter integer LOGN = 10,
    parameter integer D    = 512,   // max clique size the stack can hold
    parameter integer LOGD = 9
) (
    input  wire              clk,
    input  wire              rst,
    // host load port: one adjacency row per cycle
    input  wire              load_we,
    input  wire [LOGN-1:0]   load_addr,
    input  wire [N-1:0]      load_data,
    // job
    input  wire [LOGN:0]     n_vert,
    input  wire [LOGN:0]     init_best,
    input  wire              start,
    output reg               done,
    output reg  [LOGN:0]     best,
    output reg  [63:0]       cycles,
    output reg  [63:0]       nodes,
    output reg               overflow,   // stack depth D exceeded
    // best clique read-out
    input  wire [LOGD-1:0]   rd_addr,
    output wire [LOGN-1:0]   rd_vertex
);
    localparam [3:0] S_IDLE = 4'd0, S_CINIT = 4'd1, S_CCLASS = 4'd2, S_CPICK = 4'd3,
                     S_CUPD = 4'd4, S_CDONE = 4'd5, S_BAND = 4'd6, S_REC = 4'd7,
                     S_POP = 4'd8, S_POP2 = 4'd9, S_DONE = 4'd10;

    reg [3:0] state;

    reg [N-1:0]    adj [0:N-1];
    reg [N-1:0]    stk [0:D-1];
    reg [LOGN-1:0] rv  [0:D-1];
    reg [LOGN-1:0] brv [0:D-1];

    reg [N-1:0]    P, U, Q;
    reg [N-1:0]    adj_q, stk_q;
    reg [LOGD:0]   L;
    reg [LOGN:0]   k;
    reg [LOGN-1:0] last_v, v;
    reg [LOGD:0]   rec_i;

    assign rd_vertex = brv[rd_addr];

    // lowest set bit of Q
    wire            q_valid;
    wire [LOGN-1:0] q_idx;
    ffs #(.W(N), .LW(LOGN)) u_ffs (.x(Q), .valid(q_valid), .idx(q_idx));

    // initial candidate set: vertices 0 .. n_vert-1
    wire [N-1:0] p_init;
    genvar gi;
    generate
        for (gi = 0; gi < N; gi = gi + 1) begin : g_init
            assign p_init[gi] = (gi < n_vert);
        end
    endgenerate

    // synchronous-read address for the adjacency RAM
    wire [LOGN-1:0] adj_raddr = (state == S_CPICK) ? q_idx : last_v;
    always @(posedge clk) begin
        if (load_we) adj[load_addr] <= load_data;
        adj_q <= adj[adj_raddr];
    end

    wire [N-1:0] np = P & adj_q;
    wire [N-1:0] cont = P & ~({{(N-1){1'b0}}, 1'b1} << v);

    // stack RAM: write in S_BAND, synchronous read in S_POP
    wire [LOGD-1:0] stk_raddr = L[LOGD-1:0] - 1'b1;
    always @(posedge clk) begin
        if (state == S_BAND) begin
            stk[L[LOGD-1:0]] <= cont;
            rv[L[LOGD-1:0]]  <= v;
        end
        stk_q <= stk[stk_raddr];
        if (state == S_REC) brv[rec_i[LOGD-1:0]] <= rv[rec_i[LOGD-1:0]];
    end

    always @(posedge clk) begin
        if (rst) begin
            state <= S_IDLE; done <= 1'b0; cycles <= 0; nodes <= 0; overflow <= 1'b0;
            best <= 0; L <= 0; k <= 0; last_v <= 0; v <= 0; rec_i <= 0;
            P <= 0; U <= 0; Q <= 0;
        end else begin
            if (state != S_IDLE && state != S_DONE) cycles <= cycles + 1;
            case (state)
                S_IDLE: if (start) begin
                    P <= p_init; L <= 0; best <= init_best;
                    cycles <= 0; nodes <= 0; done <= 1'b0; overflow <= 1'b0;
                    state <= S_CINIT;
                end
                S_CINIT: begin
                    nodes <= nodes + 1;
                    U <= P; k <= 0;
                    state <= S_CCLASS;
                end
                S_CCLASS: begin
                    if (U == {N{1'b0}}) state <= S_CDONE;
                    else begin Q <= U; k <= k + 1'b1; state <= S_CPICK; end
                end
                S_CPICK: begin
                    if (!q_valid) state <= S_CCLASS;
                    else begin
                        last_v      <= q_idx;
                        Q[q_idx]    <= 1'b0;
                        U[q_idx]    <= 1'b0;
                        state       <= S_CUPD;
                    end
                end
                S_CUPD: begin
                    Q <= Q & ~adj_q;          // remove N(v) from this colour class
                    state <= S_CPICK;
                end
                S_CDONE: begin
                    if ({{(LOGN-LOGD){1'b0}}, L} + k <= best) state <= S_POP;
                    else begin v <= last_v; state <= S_BAND; end
                end
                S_BAND: begin
                    L <= L + 1'b1;
                    if (L + 1'b1 >= D) begin overflow <= 1'b1; state <= S_DONE; end
                    else if (np == {N{1'b0}}) begin
                        if ({{(LOGN-LOGD){1'b0}}, L} + 1'b1 > best) begin
                            best <= {{(LOGN-LOGD){1'b0}}, L} + 1'b1; rec_i <= 0; state <= S_REC;
                        end else state <= S_POP;
                    end else begin
                        P <= np; state <= S_CINIT;
                    end
                end
                S_REC: begin                      // copy R into the best-clique RAM
                    rec_i <= rec_i + 1'b1;
                    if (rec_i + 1'b1 == L) state <= S_POP;
                end
                S_POP: begin
                    if (L == 0) begin done <= 1'b1; state <= S_DONE; end
                    else begin L <= L - 1'b1; state <= S_POP2; end
                end
                S_POP2: begin
                    P <= stk_q;
                    state <= S_CINIT;
                end
                S_DONE: if (start) state <= S_IDLE;
                default: state <= S_IDLE;
            endcase
        end
    end
endmodule
