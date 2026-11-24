// Synthesis top: narrow 32-bit host interface around clique_engine.
// A row is shifted in 32 bits at a time and committed to adjacency RAM.
module top_ecp5 #(
    parameter integer N = 1024, LOGN = 10, D = 512, LOGD = 9
) (
    input  wire            clk,
    input  wire            rst,
    input  wire [31:0]     host_word,
    input  wire            host_shift,      // shift host_word into the row buffer
    input  wire            host_commit,     // write row buffer to adjacency row host_addr
    input  wire [LOGN-1:0] host_addr,
    input  wire [LOGN:0]   n_vert,
    input  wire [LOGN:0]   init_best,
    input  wire            start,
    output wire            done,
    output wire            overflow,
    output wire [LOGN:0]   best,
    output wire [31:0]     cycles_lo,
    input  wire [LOGD-1:0] rd_addr,
    output wire [LOGN-1:0] rd_vertex
);
    reg [N-1:0] row_buf;
    always @(posedge clk) if (host_shift) row_buf <= {host_word, row_buf[N-1:32]};

    wire [63:0] cycles, nodes;
    clique_engine #(.N(N), .LOGN(LOGN), .D(D), .LOGD(LOGD)) u_eng (
        .clk(clk), .rst(rst),
        .load_we(host_commit), .load_addr(host_addr), .load_data(row_buf),
        .n_vert(n_vert), .init_best(init_best), .start(start), .done(done), .best(best),
        .cycles(cycles), .nodes(nodes), .overflow(overflow), .rd_addr(rd_addr), .rd_vertex(rd_vertex));
    assign cycles_lo = cycles[31:0];
endmodule
