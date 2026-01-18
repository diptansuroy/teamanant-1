`timescale 1ns / 1ps
`include "waves.sv"
module targeting_system (
    input wire clk,
    input wire rst_n, // Active -low asynchronous reset
    input wire [2:0] sensor_in,
    output reg proton_fire // Set this to HIGH to fire the torpedoes.
);

    parameter [3:0] IDLE         = 4'b0000;
    parameter [3:0] CALIB_1      = 4'b0001;
    parameter [3:0] CALIB_2      = 4'b0010;
    parameter [3:0] FOUND_LEFT   = 4'b0100;
    parameter [3:0] FOUND_RIGHT  = 4'b0101;
    parameter [3:0] WINDOW_LOOP  = 4'b0111;

    reg [3:0] state;
    reg [4:0] timer;      
    reg [1:0] hit_count;  

    always @(posedge clk or negedge rst_n) begin
      
        // Asynchronous Reset Logic
        if (!rst_n) begin
            state <= IDLE;
            proton_fire <= 1'b0;
            timer <= 5'd0;
            hit_count <= 2'd0;
          
        // Other reset logic
        end else if (sensor_in == 3'b101) begin
            state <= IDLE;
            proton_fire <= 1'b0;
            timer <= 5'd0;
            hit_count <= 2'd0;
          
        end else begin
            proton_fire <= 1'b0; 
            case (state)
                IDLE: begin
                    timer <= 5'd0;
                    hit_count <= 2'd0;
                    if (sensor_in == 3'b111) state <= CALIB_1;
                end
                
                CALIB_1: begin
                    if (sensor_in == 3'b111) state <= CALIB_2;
                    else state <= IDLE;
                end
                
                CALIB_2: begin
                    if (sensor_in == 3'b001) state <= FOUND_LEFT;
                    else if (sensor_in == 3'b111) state <= IDLE; 
                    else state <= IDLE; 
                end
                
                FOUND_LEFT: begin
                    if (sensor_in == 3'b010) state <= FOUND_RIGHT;       
                end
                
                FOUND_RIGHT: begin
                    timer <= 5'd1;
                    hit_count <= 2'd0;
                    state <= WINDOW_LOOP; 
                end
                
                WINDOW_LOOP: begin
                    if (timer <= 5'd16) timer <= timer + 1;
                    if (sensor_in == 3'b100) begin 
                        hit_count <= hit_count + 1;
                        
                        if (hit_count == 2'd1) begin
                            proton_fire <= 1'b1;
                            state <= IDLE; 
                        end
                    end else if (timer > 5'd16) begin
                        state <= IDLE; 
                    end
                end
                
                default: state <= IDLE;
            endcase
        end
    end

endmodule
