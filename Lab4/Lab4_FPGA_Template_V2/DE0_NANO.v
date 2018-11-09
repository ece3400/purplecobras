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
wire HREF = GPIO_1_D[2];
wire VSYNC = GPIO_1_D[4];

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
	.clk_R(c1_sig),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],
	GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
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
	// reset y if vysnc is high or the y address is greater than screen height
	if ( ( Y_ADDR >= `SCREEN_HEIGHT - 1 ) || VSYNC ) begin
		Y_ADDR = 0;
	end
	// otherwise incrememnt y
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
				VGA_READ_MEM_EN = 1'b1;
		end
end


//downsampler
reg which_byte = 1'b0;
//reg [15:0] cameradata;

always @ (posedge PCLK) begin
	if (VSYNC) begin 
		// if vysnc is high, it's the end of the valid data
		W_EN = 0;
		X_ADDR = 0;
		which_byte = 0;
		pixel_data_RGB332[7:0] = 0;
	end
	else begin 
		// if HREF goes low, its the end of the row
		if (!HREF) begin
			W_EN = 0;
			X_ADDR = 0;
			which_byte = 1'b0;
			pixel_data_RGB332[7:0] = 0;
		end
		else begin
			if (!which_byte ) begin
				// set write enable high here so we don't write while collecting data
				W_EN = 0;
				X_ADDR = X_ADDR;
				// get data for blue
				pixel_data_RGB332[1:0] = {GPIO_1_D[16], GPIO_1_D[14]}; 
				which_byte = 1'b1;
			end
			else begin
				// get data for red and green
				pixel_data_RGB332[7:5] = {GPIO_1_D[22], GPIO_1_D[20], GPIO_1_D[18]};
				pixel_data_RGB332[4:2] = {GPIO_1_D[12], GPIO_1_D[10], GPIO_1_D[8]};
				X_ADDR = X_ADDR + 1'b1;
				which_byte = 1'b0;
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