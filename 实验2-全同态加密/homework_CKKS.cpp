#include "examples.h"
/*该文件可以在SEAL/native/example目录下找到*/
#include <vector>
using namespace std;
using namespace seal;
#define N 3
//本例目的：给定x, y, z三个数的密文，让服务器计算x*y*z

int main(){

//初始化要计算的原始数据
vector<double> x, y, z;
	x = { 1.0, 2.0, 3.0 };
	y = { 2.0, 3.0, 4.0 };
	z = { 3.0, 4.0, 5.0 };

/**********************************
客户端的视角：生成参数、构建环境和生成密文
***********************************/
//（1）构建参数容器 parms
EncryptionParameters parms(scheme_type::ckks);
/*CKKS有三个重要参数：
1.poly_module_degree(多项式模数)
2.coeff_modulus（参数模数）
3.scale（规模）*/

size_t poly_modulus_degree = 16384;
parms.set_poly_modulus_degree(poly_modulus_degree);
parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 40, 40, 40, 40, 40, 40, 60 }));
//选用2^40进行编码
double scale = pow(2.0, 40);


//（2）用参数生成CKKS框架context 
SEALContext context(parms);


//（3）构建各模块
//首先构建keygenerator，生成公钥、私钥 
KeyGenerator keygen(context); 
auto secret_key = keygen.secret_key();
PublicKey public_key;
    keygen.create_public_key(public_key);


//构建编码器，加密模块、运算器和解密模块
//注意加密需要公钥pk；解密需要私钥sk；编码器需要scale
	Encryptor encryptor(context, public_key);
	Decryptor decryptor(context, secret_key);

	CKKSEncoder encoder(context);


//对向量x、y、z进行编码
	Plaintext xp, yp, zp;
	encoder.encode(x, scale, xp);
	encoder.encode(y, scale, yp);
	encoder.encode(z, scale, zp);
//对明文xp、yp、zp进行加密
	Ciphertext xc, yc, zc;
	encryptor.encrypt(xp, xc);
	encryptor.encrypt(yp, yc);
	encryptor.encrypt(zp, zc);
 


//至此，客户端将pk、CKKS参数发送给服务器，服务器开始运算
/**********************************
服务器的视角：生成重线性密钥、构建环境和执行密文计算
***********************************/
//生成重线性密钥和构建环境
SEALContext context_server(parms);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
	Evaluator evaluator(context_server);  

/*对密文进行计算，要说明的原则是：
-加法可以连续运算，但乘法不能连续运算
-密文乘法后要进行relinearize操作
-执行乘法后要进行rescaling操作
-进行运算的密文必需执行过相同次数的rescaling（位于相同level）*/ 
	Ciphertext temp;
	Ciphertext result_c;

    //初始化一个常量
    Plaintext wt;
	encoder.encode(1.0, scale, wt);

    evaluator.multiply_plain_inplace(zc, wt);
	evaluator.rescale_to_next_inplace(zc);

    evaluator.multiply_plain_inplace(yc, wt);
	evaluator.rescale_to_next_inplace(yc);


//计算y*z，密文相乘，要进行relinearize和rescaling操作 
	evaluator.multiply(zc,yc,temp);
	evaluator.relinearize_inplace(temp, relin_keys);
	evaluator.rescale_to_next_inplace(temp);

    Ciphertext temp2;
    evaluator.multiply(xc,xc,temp2);
	evaluator.relinearize_inplace(temp2, relin_keys);
	evaluator.rescale_to_next_inplace(temp2);

    Ciphertext temp3;
    evaluator.multiply_plain_inplace(xc, wt);
	evaluator.rescale_to_next_inplace(xc);

    evaluator.multiply(temp2,xc,temp3);
	evaluator.relinearize_inplace(temp3, relin_keys);
	evaluator.rescale_to_next_inplace(temp3);


//最后执行加法
	evaluator.add_inplace(temp3, temp);
	//evaluator.relinearize_inplace(temp3,relin_keys);
	//evaluator.rescale_to_next(temp3, result_c);
    result_c=temp3;

//计算完毕，服务器把结果发回客户端
/**********************************
客户端的视角：进行解密和解码
***********************************/
//客户端进行解密
	Plaintext result_p;
	decryptor.decrypt(result_c, result_p);
//注意要解码到一个向量上
	vector<double> result;
	encoder.decode(result_p, result);
//得到结果，正确的话将输出：{6.000，24.000，60.000，...，0.000，0.000，0.000}
	cout << "结果是：" << endl;
	print_vector(result,3,3);
return 0;
}

