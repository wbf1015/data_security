from phe import paillier  # 开源库
from Cryptodome.Cipher import AES # 使用python的AES库
import random  # 选择随机数


# str不是16的倍数那就补足为16的倍数
# AES只能用16位的，但这个没用了，一开始我想把int转成str然后在弄成byte，但是这样会出现乱码
# 然后上网一查发现int和byte可以直接相互转换，小丑竟是我自己
def add_to_16(value):
    while len(value) % 16 != 0:
        value += '\0'
    return str.encode(value)  # 返回bytes


# 对称密钥，改成什么样子都行，如果以字符串形式出现的话记得弄成byte类型，16位就好
password = b'1234567812345678'

# 服务器端保存的数值
message_list = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
length = len(message_list)

# 服务端对保存的内容进行加密
encrypt_message_list = []
aes = AES.new(password, AES.MODE_ECB)  # 创建一个aes对象，采用ECB加密模式

# 对每一个message都加密然后保存的一个list中
for i in range(length):
    en_text = aes.encrypt(message_list[i].to_bytes(length=16, byteorder='big', signed=False))  # 加密明文
    encrypt_message_list.append(en_text)

# 客户端生成公私钥
public_key, private_key = paillier.generate_paillier_keypair()
# 客户端随机选择一个要读的位置
pos = random.randint(0, length - 1)
print("要读起的数值位置为：", pos)

##################### 客户端生成密文选择向量
select_list = []
enc_list = []
for i in range(length):
    select_list.append(i == pos)
    enc_list.append(public_key.encrypt(select_list[i]))

# for element in select_list:
#     print(element)
# for element in enc_list:
#     print(private_key.decrypt(element))

##################### 服务器端进行运算
c = 0
for i in range(length):
    # 根据paillier算法的要求把byte转成int进行计算
    trans = int().from_bytes(encrypt_message_list[i], byteorder='big', signed=False)
    c = c + trans * enc_list[i]
print("产生密文：", c.ciphertext())

##################### 客户端进行解密
m = private_key.decrypt(c)
# 把拿回来的byte通过AES解密成需要的int
print('未经过AES解密的密文为: ', m)
m = aes.decrypt(m.to_bytes(length=16, byteorder='big', signed=True))
m = int().from_bytes(m, byteorder='big', signed=True)
print("经过AES解密后得到数值：", m)
