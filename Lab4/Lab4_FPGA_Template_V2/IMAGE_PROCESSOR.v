//`define SCREEN_WIDTH 176
//`define SCREEN_HEIGHT 144
//`define NUM_BARS 3
//`define BAR_HEIGHT 48
//
//module IMAGE_PROCESSOR (
//	PIXEL_IN,
//	CLK,
//	VGA_PIXEL_X,
//	VGA_PIXEL_Y,
//	VGA_VSYNC_NEG,
//	RESULT
//);
//
//
////=======================================================
////  PORT declarations
////=======================================================
//input	[7:0]	PIXEL_IN;
//input			CLK;
//input [9:0]	VGA_PIXEL_X;
//input	[9:0]	VGA_PIXEL_Y;
//input			VGA_VSYNC_NEG;
//
//output reg [1:0] RESULT;
//
//reg [15:0] BLUECOUNT = 0;
//reg [15:0] REDCOUNT = 0;
//reg [15:0] REDCOUNTTEMP = 0;
//reg [15:0] BLUECOUNTTEMP = 0;
//
//always @ (posedge CLK, negedge VGA_VSYNC_NEG) begin
//		if (!VGA_VSYNC_NEG) begin
//			if (BLUECOUNTTEMP > REDCOUNTTEMP && BLUECOUNTTEMP > 16’d17000) begin
//				// set result to 2 when blue is detected
//				RESULT = 2'b10;
//			end
//			else if (REDCOUNTTEMP > BLUECOUNTTEMP && REDCOUNTTEMP > 16’d17000) begin
//				// set result to 1 when red is detected
//				RESULT = 2'b01;
//			end
//			else begin
//				// set result to 0 if no color
//				RESULT = 2'b00;
//			end
//			BLUECOUNT = 0;
//			REDCOUNT = 0;
//		end
//		else begin
//			// determine whether the majority of the pixels are red or blue
//			if (PIXEL_IN[7:6] > PIXEL_IN[1:0] && PIXEL_IN[7:6] > PIXEL_IN[4:3]) begin
//				REDCOUNT = REDCOUNT + 1'b1;
//			end
//			else if (PIXEL_IN[1:0] > PIXEL_IN[7:6] && PIXEL_IN[1:0] > PIXEL_IN[4:3]) begin
//				BLUECOUNT = BLUECOUNT + 1'b1;
//			end
//			REDCOUNTTEMP = REDCOUNT;
//			BLUECOUNTTEMP = BLUECOUNT;
//		end
//		//end
//end
//
//
//endmodule

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
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;



endmodule