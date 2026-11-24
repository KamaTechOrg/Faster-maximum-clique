// Find-first-set (lowest set bit) as a balanced tree: log2(W) levels of 2:1 muxes.
// W must be a power of two (>= 2), LW = log2(W).
module ffs #(
    parameter integer W  = 1024,
    parameter integer LW = 10
) (
    input  wire [W-1:0]  x,
    output wire          valid,
    output wire [LW-1:0] idx
);
    generate
        if (W == 2) begin : g_leaf
            assign valid = x[0] | x[1];
            assign idx   = x[0] ? 1'b0 : 1'b1;
        end else begin : g_node
            wire          lo_v, hi_v;
            wire [LW-2:0] lo_i, hi_i;
            ffs #(.W(W/2), .LW(LW-1)) u_lo (.x(x[W/2-1:0]), .valid(lo_v), .idx(lo_i));
            ffs #(.W(W/2), .LW(LW-1)) u_hi (.x(x[W-1:W/2]), .valid(hi_v), .idx(hi_i));
            assign valid = lo_v | hi_v;
            assign idx   = lo_v ? {1'b0, lo_i} : {1'b1, hi_i};
        end
    endgenerate
endmodule
