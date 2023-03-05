from Cryptodome.Cipher import AES

# str不是16的倍数那就补足为16的倍数
def add_to_16(value):
    while len(value) % 16 != 0:
        value += '\0'
    return str.encode(value)  # 返回bytes

password = b'1234567812345678'  # 秘钥，b就是表示为bytes类型
text = 100  # 需要加密的内容，bytes类型

str_text = (str(text))
# print(str_text)

aes = AES.new(password, AES.MODE_ECB)  # 创建一个aes对象
# AES.MODE_ECB 表示模式是ECB模式


en_text = aes.encrypt(add_to_16(str_text))  # 加密明文
print("密文：", en_text)  # 加密明文，bytes类型
den_text = aes.decrypt(en_text)  # 解密密文
print("明文：", den_text)

ans = den_text.decode('UTF-8')
print(ans)

print()
