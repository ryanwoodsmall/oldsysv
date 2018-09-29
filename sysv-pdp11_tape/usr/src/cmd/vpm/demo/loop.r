#	loop.r 1.1 of 7/19/81
#	@(#)loop.r	1.1
	array rtype[3];
	array xtype[3];
function loop()
	repeat {
		trace(0377);
		while (getxbuf(xtype) != 0);
		++nxbuf;
		trace(0376);
		while (getrbuf(rtype) != 0);
		trace(0375);
		++nrbuf;
		while(get(byte) == 0) 
			put(byte);
snap(xtype);
snap(rtype);
		xtype[0]=020;
		xtype[1]=021;
		xtype[2]=022;
		rtype[0]=030;
		rtype[1]=031;
		rtype[2]=032;
		rtnrbuf(rtype);
		rtnxbuf(xtype);
		snap(xtype);
		snap(rtype);
	}
end
