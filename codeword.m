clear
close all

file='T_BG1_26_4_48.txt';
fid=fopen(['./',file],'r');
N=fscanf(fid,'%d',1);
M=fscanf(fid,'%d',1);%N=K+M;
K=N-M;
max_vn_weight=fscanf(fid,'%d',1);
max_cn_weight=fscanf(fid,'%d',1);
VN_weight=fscanf(fid,'%d',N);
CN_weight=fscanf(fid,'%d',M);
H=zeros(M,N);

rowid=[];
colid=[];
for i=1:N
    for t=1:VN_weight(i)
        a=fscanf(fid,'%d',1);
        rowid=[rowid,a];
        H(a,i)=1;
        colid=[colid,i];
    end
end

fclose(fid);

S=sparse(rowid,colid,1);
ldpcencoder=comm.LDPCEncoder(S);
message=round(rand(K,1));
encode_codeword=ldpcencoder(message);

check=mod(H*encode_codeword,2);
if(sum(check(:)==1)~=0)
    print('error');
end

fid=fopen(sprintf('./codeword_%d.txt',N),'w');

fprintf(fid,'int CodeWord_sym[%d]={',N);

for i=1:N-1
    fprintf(fid,'%d,',encode_codeword(i));
end

fprintf(fid,'%d};',encode_codeword(N));
fclose(fid);


