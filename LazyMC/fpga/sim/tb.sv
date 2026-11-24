// Testbench: loads <prefix>.adj.hex / <prefix>.cfg written by mc_model --emit-hex,
// runs clique_engine, and checks best size, cycle count, node count and that the
// returned vertex set is a clique.
`timescale 1ns/1ps
module tb;
    localparam integer N = 1024, LOGN = 10, D = 512, LOGD = 9;

    reg clk = 0, rst = 1;
    always #5 clk = ~clk;

    reg              load_we = 0;
    reg  [LOGN-1:0]  load_addr = 0;
    reg  [N-1:0]     load_data = 0;
    reg  [LOGN:0]    n_vert = 0, init_best = 0;
    reg              start = 0;
    wire             done, overflow;
    wire [LOGN:0]    best;
    wire [63:0]      cycles, nodes;
    reg  [LOGD-1:0]  rd_addr = 0;
    wire [LOGN-1:0]  rd_vertex;

    clique_engine #(.N(N), .LOGN(LOGN), .D(D), .LOGD(LOGD)) dut (
        .clk(clk), .rst(rst), .load_we(load_we), .load_addr(load_addr), .load_data(load_data),
        .n_vert(n_vert), .init_best(init_best), .start(start), .done(done), .best(best),
        .cycles(cycles), .nodes(nodes), .overflow(overflow), .rd_addr(rd_addr), .rd_vertex(rd_vertex));

    reg [N-1:0] rows [0:N-1];
    reg [LOGN-1:0] clique [0:D-1];
    string prefix;
    integer fd, n, ib, exp_best, i, j, errors;
    longint exp_cycles, exp_nodes;

    initial begin
        if (!$value$plusargs("prefix=%s", prefix)) begin $display("need +prefix="); $finish; end
        fd = $fopen({prefix, ".cfg"}, "r");
        void'($fscanf(fd, "%d %d %d %d %d", n, ib, exp_best, exp_cycles, exp_nodes));
        $fclose(fd);
        for (i = 0; i < N; i++) rows[i] = '0;
        $readmemh({prefix, ".adj.hex"}, rows, 0, n - 1);

        repeat (2) @(posedge clk);
        rst <= 0;
        for (i = 0; i < n; i++) begin             // host -> FPGA: one row per cycle
            @(posedge clk);
            load_we <= 1; load_addr <= i[LOGN-1:0]; load_data <= rows[i];
        end
        @(posedge clk); load_we <= 0;
        n_vert <= n[LOGN:0]; init_best <= ib[LOGN:0];
        @(posedge clk); start <= 1;
        @(posedge clk); start <= 0;
        wait (done);
        @(posedge clk);

        errors = 0;
        if (overflow) begin $display("OVERFLOW"); errors++; end
        if (best != exp_best[LOGN:0]) begin $display("best mismatch: rtl=%0d model=%0d", best, exp_best); errors++; end
        if (cycles != exp_cycles) begin $display("cycle mismatch: rtl=%0d model=%0d", cycles, exp_cycles); errors++; end
        if (nodes != exp_nodes) begin $display("node mismatch: rtl=%0d model=%0d", nodes, exp_nodes); errors++; end
        if (best > ib[LOGN:0]) begin
            for (i = 0; i < best; i++) begin
                rd_addr <= i[LOGD-1:0]; @(posedge clk); #1 clique[i] = rd_vertex;
            end
            for (i = 0; i < best; i++)
                for (j = i + 1; j < best; j++)
                    if (!rows[clique[i]][clique[j]]) begin
                        $display("not a clique: %0d-%0d", clique[i], clique[j]); errors++;
                    end
        end
        $display("RESULT n=%0d init_best=%0d best=%0d nodes=%0d cycles=%0d %s",
                 n, ib, best, nodes, cycles, errors == 0 ? "PASS" : "FAIL");
        $finish;
    end
endmodule
