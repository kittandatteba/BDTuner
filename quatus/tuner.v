module tuner(FX2_SLOE, FX2_SLRD, FX2_SLWR, FX2_FD, FX2_FIFOADR, FX2_PKTEND,
		FX2_FLAGA, FX2_FLAGB, FX2_FLAGC, FX2_IFCLK, 
		TS_SCLK, TS_SD, TS_SBYTE, TS_PBVAL,TS_ERR,
		TS_JSCLK,TS_JSD,TS_JSBYTE,TS_JPBVAL,TS_JERR,
		TS_PORT
		);

output			FX2_SLOE;
output			FX2_SLRD;
output			FX2_SLWR;
inout	[15:0]	FX2_FD;
output [1:0]	FX2_FIFOADR;
output			FX2_PKTEND;
input				FX2_FLAGA;
input				FX2_FLAGB;
input				FX2_FLAGC;
input				FX2_IFCLK;

input			TS_SCLK;
input			TS_SD;
input			TS_SBYTE;
input			TS_PBVAL;
input			TS_ERR;
input			TS_JSCLK;
input			TS_JSD;
input			TS_JSBYTE;
input			TS_JPBVAL;
input			TS_JERR;
input			TS_PORT;

wire [15:0]	ts_data_t;
wire [15:0]	ts_data_s;
wire			ts_req_t;
wire			ts_req_s;
wire			ts_req_clr_t;
wire			ts_req_clr_s;
ser2par		ser2par_t(TS_SCLK, TS_SD, TS_PBVAL, ts_data_t, ts_req_t, ts_req_clr_t);
ser2par		ser2par_s(TS_JSCLK, TS_JSD, TS_JPBVAL, ts_data_s, ts_req_s, ts_req_clr_s);

reg			slwr_reg;
reg			ts_clr_reg_t;
reg			ts_clr_reg_s;
reg			t_s_sw;
reg [1:0]	_wait;
reg			_ts_req_t;
reg			_ts_req_s;

always@ (posedge FX2_IFCLK) begin
	_ts_req_t <= ts_req_t;
	_ts_req_s <= ts_req_s;
	if((_ts_req_t == 1) && (_wait == 0)) begin
		ts_clr_reg_t <= 1;
		t_s_sw <= 0;
		_wait <= 1;
	end else if((_ts_req_s == 1) && (_wait == 0)) begin
		ts_clr_reg_s <= 1;
		t_s_sw <= 1;
		_wait <= 1;
	end

	if(_wait == 1) begin
		slwr_reg <= 0;
		_wait <= 2;
	end
	if(_wait == 2) begin
		slwr_reg <= 1;
		ts_clr_reg_t <= 0;
		ts_clr_reg_s <= 0;
		_wait <= 0;
	end
end

assign ts_req_clr_t = ts_clr_reg_t;
assign ts_req_clr_s = ts_clr_reg_s;

assign FX2_SLOE = 1'b1;
assign FX2_FIFOADR = (t_s_sw == 0) ? 2'b00 : 2'b10;
assign FX2_PKTEND = 1'b1;
assign FX2_SLRD = 1'b1;
assign FX2_SLWR = (t_s_sw == 0) ? ((FX2_FLAGA == 1) ? slwr_reg : 1'b1) : ((FX2_FLAGB == 1) ? slwr_reg : 1'b1);
assign FX2_FD = (t_s_sw == 0) ? ts_data_t : ts_data_s;

endmodule
