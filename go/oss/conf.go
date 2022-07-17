package oss

const (
	endPoint                = "oss-cn-hangzhou.aliyuncs.com"
	dayMini1AccessKeyID     = "LTAI5tHEvf1cAgf4miA7vL1Z"
	dayMini1AccessKeySecret = "nkUIDWGLi5hXey5Mnp4CnuXXvyEFpn"
	dayMini2AccessKeyID     = "LTAI5tJBjnh4rExGGb1DBzM9"
	dayMini2AccessKeySecret = "l3VQakBcnGKSdElGCydobJZKIxeQDq"
	bucketName              = "daydaylw3"
)

var (
	dayMini1 = accessKey{
		id:     "LTAI5tHEvf1cAgf4miA7vL1Z",
		secret: "nkUIDWGLi5hXey5Mnp4CnuXXvyEFpn",
	}
	dayMini2 = accessKey{
		id:     "LTAI5tJBjnh4rExGGb1DBzM9",
		secret: "l3VQakBcnGKSdElGCydobJZKIxeQDq",
	}
)

type accessKey struct {
	id     string
	secret string
}
