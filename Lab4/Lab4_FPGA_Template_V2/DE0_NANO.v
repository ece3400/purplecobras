//`define SCREEN_WIDTH 176
//`define SCREEN_HEIGHT 144
//
/////////* DON'T CHANGE THIS PART *///////
//module DE0_NANO(
//	CLOCK_50,
//	GPIO_0_D,
//	GPIO_1_D,
//	KEY
//);
//
////=======================================================
////  PARAMETER declarations
////=======================================================
//localparam RED = 8'b111_000_00;
//localparam GREEN = 8'b000_111_00;
//localparam BLUE = 8'b000_000_11;
//
////=======================================================
////  PORT declarations
////=======================================================
//
////////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
//input 		          		CLOCK_50;
//
////////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
//output 		    [33:0]		GPIO_0_D;
////////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
//input 		    [33:0]		GPIO_1_D;
//input 		     [1:0]		KEY;
//
/////// PIXEL DATA /////
//reg [7:0]	pixel_data_RGB332 = 8'd0;
//
/////// READ/WRITE ADDRESS /////
//reg [14:0] X_ADDR;
//reg [14:0] Y_ADDR;
//wire [14:0] WRITE_ADDRESS;
//reg [14:0] READ_ADDRESS; 
//
//assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);
//
////wire			C0_to_GPIO_02;
//wire			PLL_24;
//wire			PLL_50;
//wire			PLL_25;
//
//wire			PCLK;
//wire			HREF;
//wire			VSYNC;
//
//// Camera stuff
//assign GPIO_0_D[2] = HREF;
//assign GPIO_0_D[4] = VSYNC;
//assign GPIO_0_D[6] = PCLK;
//
//reg  [1:0]  which_byte;
//
//wire 			camera_0;
//wire 			camera_1;
//wire 			camera_2;
//wire 			camera_3;
//wire 			camera_4;
//wire 			camera_5;
//wire 			camera_6;
//wire 			camera_7;
//
//assign camera_0 = GPIO_1_D[8];
//assign camera_1 = GPIO_1_D[10];
//assign camera_2 = GPIO_1_D[12];
//assign camera_3 = GPIO_1_D[14];
//assign camera_4 = GPIO_1_D[16];
//assign camera_5 = GPIO_1_D[18];
//assign camera_6 = GPIO_1_D[20];
//assign camera_7 = GPIO_1_D[22];
//
//reg  [7:0]  color_temp;
//reg  [2:0]	temp_counter;
//reg 			reset_temp_counter;
//
//
/////// VGA INPUTS/OUTPUTS /////
//wire 			VGA_RESET;
//wire [7:0]	VGA_COLOR_IN;
//wire [9:0]	VGA_PIXEL_X;
//wire [9:0]	VGA_PIXEL_Y;
//wire [7:0]	MEM_OUTPUT;
//wire			VGA_VSYNC_NEG;
//wire			VGA_HSYNC_NEG;
//reg			VGA_READ_MEM_EN;
//
//assign GPIO_0_D[5] = VGA_VSYNC_NEG;
//assign VGA_RESET = ~KEY[0];
//
/////// I/O for Img Proc /////
//wire [8:0] RESULT;
//
///* WRITE ENABLE */
//reg W_EN;
//
/////////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
//
/////////* INSTANTIATE YOUR PLL HERE *///////
//sweetPLL	sweetPLL_inst (
//	.inclk0 ( CLOCK_50 ),
//	.c0 ( PLL_24 ),
//	.c1 ( PLL_25 ),
//	.c2 ( PLL_50 )
//	);
//	
//assign GPIO_0_D[33] = PLL_24;
//
/////////* M9K Module *///////
//Dual_Port_RAM_M9K mem(
//	.input_data(pixel_data_RGB332),
//	.w_addr(WRITE_ADDRESS),
//	.r_addr(READ_ADDRESS),
//	.w_en(W_EN),
//	.clk_W(PLL_50),
//	.clk_R(PLL_25), // DO WE NEED TO READ SLOWER THAN WRITE??
//	.output_data(MEM_OUTPUT)
//);
//
/////////* VGA Module *///////
//VGA_DRIVER driver (
//	.RESET(VGA_RESET),
//	.CLOCK(PLL_25),
//	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
//	.PIXEL_X(VGA_PIXEL_X),
//	.PIXEL_Y(VGA_PIXEL_Y),
//	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
//   .H_SYNC_NEG(GPIO_0_D[7]),
//   .V_SYNC_NEG(VGA_VSYNC_NEG)
//);
//
/////////* Image Processor *///////
//IMAGE_PROCESSOR proc(
//	.PIXEL_IN(MEM_OUTPUT),
//	.CLK(PLL_25),
//	.VGA_PIXEL_X(VGA_PIXEL_X),
//	.VGA_PIXEL_Y(VGA_PIXEL_Y),
//	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
//	.RESULT(RESULT)
//);
//
//
///////* Update Read Address *///////
//always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
//	READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
//	if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
//			VGA_READ_MEM_EN = 1'b0;
//	end
//	else begin
//			VGA_READ_MEM_EN = 1'b1;
//	end
//end
//
////// update x, y coordinates
////always @ ( posedge PLL_50 ) begin
////	// this is the only time we want to update
////	//if ( temp_counter == 2'd2 ) begin
////		if ( HREF == 1'd0 ) begin
////			Y_ADDR <= Y_ADDR + 15'd1;
////			X_ADDR <= 15'd0;
////		end
////		else if ( Y_ADDR == `SCREEN_HEIGHT ) begin
////			Y_ADDR <= 15'd0;
////		end
////		else begin
////			X_ADDR <= X_ADDR + 15'd1;
////		end
//	//end
//	//else begin
//	//end
//
//
////	if ( X_ADDR == `SCREEN_WIDTH ) begin 
////		Y_ADDR <= Y_ADDR + 15'd1;
////		X_ADDR <= 15'd0;
////		end
////	else if ( Y_ADDR < `SCREEN_HEIGHT )begin
////		X_ADDR <= X_ADDR + 15'd1;
////	end
////	else begin
////	end
////end
//
//
//// read
//always @ ( posedge PCLK ) begin
//	if ( VSYNC ) begin
//		W_EN = 0;
//		// reset X and pixel data
//		pixel_data_RGB332[7:0] = 0;
//		X_ADDR = 0;
//		which_byte = 1'b0;
//	end
//	else begin
//		if ( !HREF ) begin
//			W_EN = 0;
//			which_byte = 1'b1;
//			pixel_data_RGB332[1:0] = { camera_3, camera_4 };
//		end
//		else begin
//			W_EN = 0;
//			which_byte = 1'b0;
//			pixel_data_RGB332[7:2] = { camera_7, camera_6, camera_5, camera_2, camera_1, camera_0 };
//			X_ADDR = X_ADDR + 1'b1;
//			W_EN = 1;
//		end
//	end
//end
//	
////	W_EN <= 1'b0;
////	if ( HREF ) begin
////		if ( which_byte == 1'd0 ) begin	
////			if ( temp_counter == 2'd0 ) begin
////				// red color
////				color_temp[7] <= 1'd1;//camera_7;
////				color_temp[6] <= 1'd1;//camera_6;
////				color_temp[5] <= 1'd1;//camera_5;
////				
////				// green color
////				color_temp[4] <= 1'd0;//camera_2;
////				color_temp[3] <= 1'd0;//camera_1;
////				color_temp[2] <= 1'd0;//camera_0;
////				which_byte <= 1'd1;
////				temp_counter <= 2'd1;
////			end	
////			else begin
////			
////			end
////		end
////		
////		// second byte
////		else begin
////			if ( temp_counter == 2'd1 ) begin
////				color_temp[1] <= 1'd0;//camera_4;
////				color_temp[0] <= 1'd0;//camera_3;
////				which_byte <= 1'd0;
////				temp_counter <= 2'd1;
////			end
////			else begin
////			end
////		end
////	end
////	else begin
////	end
////	W_EN <= 1'b1;
////end
////
////always @ ( PLL_50 ) begin
////	//if ( temp_counter == 2'd1 ) begin
////	if ( W_EN ) begin
////		pixel_data_RGB332 <= color_temp;
////		
////	end
////	else begin
////	end
////		//temp_counter <= 2'd0;
////	//end
////	//else begin
////	//end
////end
//
////// starts at the second byte
////always @ ( HREF, negedge PCLK, which_byte ) begin
////	if ( temp_counter == 2'd1 ) begin
////		color_temp[1] <= camera_4;
////		color_temp[0] <= camera_3;
////		which_byte <= 1'd0;
////		temp_counter <= 2'd2;
////	end
////end
////
////// starts at the end of the two bytes
////always @ ( negedge PCLK ) begin
////	if ( temp_counter == 2'd2 ) begin
////		pixel_data_RGB332 <= color_temp;
////		temp_counter <= 2'd0;
////	end
////end
//
//
//	
//endmodule 

`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

module DE0_NANO(
   CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY,
	RESULT
);





//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

wire c0_sig;
wire c1_sig;
wire c2_sig;

sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ), //24
	.c1 ( c1_sig ), //25
	.c2 ( c2_sig )  //50 - phase synchronized with c0 and c1  
	);


//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;

//////////// CLOCK //////////
input CLOCK_50;

wire PCLK = GPIO_1_D[6];
//wire HREF = GPIO_1_D[30];
wire HREF = GPIO_1_D[2];
wire VSYNC = GPIO_1_D[4];
//wire VSYNC = GPIO_1_D[31];

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];
assign GPIO_0_D[33] = c0_sig;


///// I/O for Img Proc /////
output wire [1:0] RESULT;

/* WRITE ENABLE */
reg W_EN;


///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(c2_sig),
	.clk_R(c1_sig), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);


	


///////* Image Processor *///////

IMAGE_PROCESSOR proc (
	.PIXEL_IN (MEM_OUTPUT),
	.CLK(c1_sig),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


always @ (negedge HREF) begin
	if ( ( Y_ADDR >= `SCREEN_HEIGHT - 1 ) || VSYNC ) begin
		Y_ADDR = 0;
	end
	else begin
		Y_ADDR = Y_ADDR + 1;
	end
end


///////* Update Read Address *///////
//buffer reader
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if((VGA_PIXEL_X>`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1)) begin 
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
//				if (VGA_PIXEL_X==VGA_PIXEL_Y) begin
//					pixel_data_RGB332 = RED;
//				end
//				else begin
//					pixel_data_RGB332 = GREEN;
//				end
				VGA_READ_MEM_EN = 1'b1;
		end
end


//downsampler
reg cycle = 1'b0;
//reg [15:0] cameradata;

always @ (posedge PCLK) begin
	if (VSYNC) begin 
		W_EN = 0;
		X_ADDR = 0;
		//Y_ADDR = 0;
		cycle = 0;
		pixel_data_RGB332[7:0] = 0;
		//cameradata[15:0] = 0;
	end
	else begin 
		if (!HREF) begin
			W_EN = 0;
			X_ADDR = 0;
			cycle = 1'b0;
			pixel_data_RGB332[7:0] = 0;
			//cameradata[15:0] = 0;
		end
		else begin
			if (!cycle ) begin
				//cameradata[15:0] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13], GPIO_1_D[12], GPIO_1_D[11], GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				W_EN = 0;
				X_ADDR = X_ADDR;
				//pixel_data_RGB332[7:5] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13]};
				//pixel_data_RGB332[4:2] = {GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				pixel_data_RGB332[1:0] = {GPIO_1_D[16], GPIO_1_D[14]}; //something is wrong with the cycles, blue being output before red/green, but this code works
				cycle = 1'b1;
			end
			else begin
				//cameradata[15:8] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13], GPIO_1_D[12], GPIO_1_D[11], GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				//pixel_data_RGB332[7:0] = {cameradata[4], cameradata[3], cameradata[2], cameradata[10], cameradata[9], cameradata[8], cameradata[15], cameradata[14]};
				pixel_data_RGB332[7:5] = {GPIO_1_D[22], GPIO_1_D[20], GPIO_1_D[18]};
				pixel_data_RGB332[4:2] = {GPIO_1_D[12], GPIO_1_D[10], GPIO_1_D[8]};
				X_ADDR = X_ADDR + 1'b1;
				cycle = 1'b0;
//				if ( X_ADDR >= `SCREEN_WIDTH ) begin
//					X_ADDR = X_ADDR;
//				end
//				else begin
//					X_ADDR = X_ADDR + 1'b1;
//				end
				//X_ADDR = X_ADDR + 1'b1;
				W_EN = 1;
			end
			
//assign camera_0 = GPIO_1_D[8];
//assign camera_1 = GPIO_1_D[10];
//assign camera_2 = GPIO_1_D[12];
//assign camera_3 = GPIO_1_D[14];
//assign camera_4 = GPIO_1_D[16];
//assign camera_5 = GPIO_1_D[18];
//assign camera_6 = GPIO_1_D[20];
//assign camera_7 = GPIO_1_D[22];
		end
	end
end

	
endmodule 