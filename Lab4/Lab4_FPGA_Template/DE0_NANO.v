`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144


module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
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

//////////// CLOCK //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		    [1:0]		KEY;
//////////// GPIO_0, GPIO_2 connect to c0 //////////
//output          GPIO_02;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

//wire			C0_to_GPIO_02;
wire			PLL_24;
wire			PLL_50;
wire			PLL_25;

wire			PCLK;
wire			HREF;
wire			VSYNC;


// Camera stuff
assign GPIO_0_D[27] = HREF;
assign GPIO_0_D[29] = VSYNC;
assign GPIO_0_D[31] = PCLK;

reg  [1:0]  which_byte;

wire 			camera_0;
wire 			camera_1;
wire 			camera_2;
wire 			camera_3;
wire 			camera_4;
wire 			camera_5;
wire 			camera_6;
wire 			camera_7;

assign camera_0 = GPIO_1_D[8];
assign camera_1 = GPIO_1_D[10];
assign camera_2 = GPIO_1_D[12];
assign camera_3 = GPIO_1_D[14];
assign camera_4 = GPIO_1_D[16];
assign camera_5 = GPIO_1_D[18];
assign camera_6 = GPIO_1_D[20];
assign camera_7 = GPIO_1_D[22];

reg  [7:0]  color_temp;
reg  [2:0]	temp_counter;

//wire [2:0]  temp_counter_wire;

//assign temp_counter_wire = temp_counter;


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

assign GPIO_0_D[33] = PLL_24;

// assign to the GPIO pin the value of the 24 MHz clock
//assign C0_to_GPIO_02 = c0;

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN;


sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( PLL_24 ),
	.c1 ( PLL_25 ),
	.c2 ( PLL_50 )
	);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(PLL_50),
	.clk_R(PLL_25), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(PLL_25),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(PLL_25),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


/////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
	READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
	if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
			VGA_READ_MEM_EN = 1'b0;
	end
	else begin
			VGA_READ_MEM_EN = 1'b1;
	end
end

always @ (posedge PLL_50) begin
	W_EN <= 1'b0;
	if ( X_ADDR == `SCREEN_WIDTH ) begin 
		Y_ADDR <= Y_ADDR + 15'd1;
		X_ADDR <= 15'd0;
		end
	else if ( Y_ADDR < `SCREEN_HEIGHT )begin
		X_ADDR <= X_ADDR + 15'd1;
	end
	else begin
	end
	W_EN <= 1'b1;
end

//always @ (X_ADDR, Y_ADDR) begin
//	if ( X_ADDR == `SCREEN_WIDTH /2 ) begin
//		pixel_data_RGB332 <= BLUE;
//		end
//	else begin
//		pixel_data_RGB332 <= GREEN;
//		end
//end	



// this will start on the first byte
always @ ( posedge HREF, negedge PCLK, !which_byte ) begin
	if ( temp_counter == 2'd0 ) begin
		// red color
		color_temp[7] <= camera_7;
		color_temp[6] <= camera_6;
		color_temp[5] <= camera_5;
		
		// green color
		color_temp[4] <= camera_2;
		color_temp[3] <= camera_1;
		color_temp[2] <= camera_0;
		which_byte <= 1'd1;
		temp_counter <= 2'd1;
	end
	else if (  ) begin
	
	end
	
	else begin
	
	end
end

// starts at the second byte
always @ ( HREF, negedge PCLK, which_byte ) begin
	if ( temp_counter == 2'd1 ) begin
		color_temp[1] <= camera_4;
		color_temp[0] <= camera_3;
		which_byte <= 1'd0;
		temp_counter <= 2'd2;
	end
end

// starts at the end of the two bytes
always @ ( negedge PCLK ) begin
	if ( temp_counter == 2'd2 ) begin
		pixel_data_RGB332 <= color_temp;
		temp_counter <= 2'd0;
	end
end

endmodule 