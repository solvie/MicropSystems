		AREA text,CODE,READONLY
		EXPORT IIR_asm
IIR_asm
		;input array R0
		;output array R1
		;length of array R2
		;coeff R3
		ADD R6,R3,#12 ; R6 starting point of coefficient b
		MOV R7,#0 ; counter i in bytes
		
loop 
		CMP R2,#0
		BEQ exit
		VLDR.f32 S3,[R6] ; (coeff->b)[0]
		;temp R8
		ADD R8,R0,R7
		VLDR.f32 S4,[R8] ; load i th element of input array
		VMUL.f32 S3,S3,S4 ; temp result=(coeff->b)[0]*InputArray[i]
		MOV R9,#4
		
inner_loop
		CMP R9,#12 ; (Order+1)*4 bytes
		BEQ end_inner_loop
		CMP R9,R7
		BGT end_inner_loop
		SUB R10,R7,R9
		;temp R11
		
		ADD R11,R6,R9
		VLDR.f32 S4,[R11] ; load b[j] into S4
		ADD R11,R0,R10
		VLDR.f32 S5,[R11] ; load InputArray[i-j] into S5
		VMUL.f32 S4,S4,S5 ; (coeff->b)[j]*InputArray[i-j]
		VADD.f32 S3,S3,S4 ; tempResult= tempResult + (coeff->b)[j]*InputArray[i-j]

		ADD R11,R3,R9
		VLDR.f32 S4,[R11] ; load a[j] into S4
		ADD R11,R1,R10
		VLDR.f32 S5,[R11] ; load OutputArray[i-j] into S5
		VMUL.f32 S4,S4,S5 ; (coeff->a)[j]*OutPutArray[i-j]
		VADD.f32 S3,S3,S4 ; tempResult= tempResult + (coeff->a)[j]*OutPut[i-j]
		
		ADD R9,R9,#4
		B inner_loop
			
end_inner_loop
		ADD R12,R1,R7
		VSTR S3,[R12]
		ADD R7,R7,#4
		SUB R2,R2,#1
		B loop
		
exit
		BX LR;
		END
