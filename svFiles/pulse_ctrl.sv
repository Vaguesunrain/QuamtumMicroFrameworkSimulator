module pulse_ctrl (
    input  logic        clk,
    input  logic        rst_n,
    input  logic        trigger,
    input  logic [7:0]  amplitude,
    output logic        pulse_active,
    output logic [7:0]  out_val
);
    logic [2:0] counter;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            counter      <= 3'b0;
            pulse_active <= 1'b0;
            out_val      <= 8'h00;
        end else if (trigger && !pulse_active) begin
            counter      <= 3'd4; // 持续4个周期
            pulse_active <= 1'b1;
            out_val      <= amplitude;
        end else if (counter > 3'd1) begin
            counter      <= counter - 1'b1;
        end else if (counter == 3'd1) begin
            counter      <= 3'd0;
            pulse_active <= 1'b0;
            out_val      <= 8'h00;
        end
    end
endmodule
