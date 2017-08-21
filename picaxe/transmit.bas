symbol SHORT_PAUSE_LOW = 140
symbol SHORT_PAUSE_HIGH = 1
symbol LONG_PAUSE = 300

setfreq m32
pullup on

main:
	high C.4
	
	if pinC.1 = 0 then
		sertxd ("Sending ... ")

		bptr = 28
		b0 = 0xBB
		gosub writebyte
		
		b0 = 8
		gosub send

		sertxd ("done", 13, 10)

		do
			pause 10
		loop while pinC.1 = 0

	endif
	pause 10
goto main

writebyte:
	@bptrinc = bit0
	@bptrinc = bit1
	@bptrinc = bit2
	@bptrinc = bit3
	@bptrinc = bit4
	@bptrinc = bit5
	@bptrinc = bit6
	@bptrinc = bit7
	
	return
	
send:
	b1 = b0
	bptr = 28
	
sendnext:
	if b1 = 0 then
		return
	endif
	
	b2 = @bptrinc
	if b2 = 1 then
		low C.4
		pauseus SHORT_PAUSE_LOW
		high C.4
		pauseus SHORT_PAUSE_HIGH
	else
		pauseus LONG_PAUSE
	endif
	
	dec b1
	goto sendnext
