#	tcmd.r 1.2 of 9/24/81
#	@(#)tcmd.r	1.2
array buf[4];
function cmd()
repeat{
	trace(50);
	while(getcmd(buf) == 0);
	trace(buf[0]);
	trace(buf[1]);
	trace(buf[2]);
	trace(buf[3]);
	trace(buf[0],buf[1]);
	trace(buf[2],buf[3]);
	snap(buf);
	buf[0] = 4;
	buf[1] = 5;
	buf[2] = 6;
	buf[3] = 7;
	snap(buf);
	rtnrpt(buf);
	for(n=0;n<10;++n){
		buf[0] = n;
		buf[1] = n;
		buf[2] = n;
		buf[3] = n;
		snap(buf);
		timer(10);
		while(timer(0));
	}
}
end
