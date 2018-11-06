`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input			CLK;
input [9:0]	VGA_PIXEL_X;
input	[9:0]	VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output reg [1:0] RESULT;

reg [9:0] BLUECOUNT;
reg [9:0] REDCOUNT;

always @ (posedge CLK) begin
		if (!VGA_VSYNC_NEG) begin
			if (BLUECOUNT > REDCOUNT && BLUECOUNT > 10'b0001100100) begin //100 is arbitrary, can change
				RESULT = 2'b01; //when color is 01, blue
			end
			else if (REDCOUNT > BLUECOUNT && REDCOUNT > 10'b0001100100) begin
				RESULT = 2'b10; //when color is 10, red
			end
			else begin
				RESULT = 2'b00; //no color detected
			end
			BLUECOUNT = 0;
			REDCOUNT = 0;
		end
		else begin
			if (VGA_PIXEL_Y >= 10'b0001000110 && VGA_PIXEL_Y <= 10'b0001001010) begin
				if (PIXEL_IN[7:6] > PIXEL_IN[1:0] && PIXEL_IN[7:6] > PIXEL_IN[4:3]) begin
					REDCOUNT = REDCOUNT + 1'b1;
				end
				else if (PIXEL_IN[1:0] > PIXEL_IN[7:6] && PIXEL_IN[1:0] > PIXEL_IN[4:3]) begin
					BLUECOUNT = BLUECOUNT + 1'b1;
				end
			end
		end
end


endmodule