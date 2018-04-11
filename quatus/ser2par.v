module ser2par(SCLK, SD, PBVAL, TS_DATA, REQ, REQ_CLR);
	input SCLK;
	input SD;
	input PBVAL;
	
	output [15:0] TS_DATA;
	output REQ;
	input  REQ_CLR;
	
	reg [7:0] pbval;
	reg [2:0] bits;
	reg [7:0] sreg;
	reg [7:0] dout0;
	reg [7:0] dout1;
	reg [7:0] dout2;
	reg [7:0] dout3;
	reg [1:0] dout_wsw;
	reg dout_rsw;
	reg req;
	
	always@(negedge SCLK or posedge REQ_CLR)
	begin
		if(REQ_CLR == 1)
			req <= 0;
		else begin
			if((req == 0) && (dout_rsw != dout_wsw[1]))
			begin
				req <= 1;
				dout_rsw <= dout_rsw + 1;
			end
		end
	end
	
	always@(negedge SCLK)
	begin
		pbval[0] <= PBVAL;
		pbval[7:1] <= pbval[6:0];

		if((pbval[7] == 0) && (PBVAL == 0))
			bits <= 0;
	
		else begin
			sreg[0] <= SD;
			sreg[7:1] <= sreg[6:0];
			bits <= bits + 1;
		end
			
		if((bits == 0)&&(pbval[7] == 1))
		begin
			if(dout_wsw == 0)
				dout0[7:0] <= sreg[7:0];
			else if(dout_wsw == 1)
				dout1[7:0] <= sreg[7:0];
			else if(dout_wsw == 2)
				dout2[7:0] <= sreg[7:0];
			else if(dout_wsw == 3)
				dout3[7:0] <= sreg[7:0];
			dout_wsw <= dout_wsw + 1;
		end
	end
	
	assign TS_DATA = (dout_rsw == 1) ? {dout1, dout0} : {dout3, dout2};
	assign REQ = req;
endmodule
