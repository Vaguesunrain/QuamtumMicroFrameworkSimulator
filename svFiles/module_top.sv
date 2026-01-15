module module_top (
    input  logic        clk,
    input  logic        rst_n,
    output logic   trigger
);
    logic [7:0] counter;

    always_ff @(posedge clk or negedge rst_n) begin
       counter <= rst_n ? counter + 1 : 8'd0;
       if (counter==8'd20) begin
          trigger <= 1'b1;
       end else begin
          trigger <= 1'b0;
       end
    end
endmodule
