package oss

import (
	"testing"

	"github.com/aliyun/aliyun-oss-go-sdk/oss"
)

// daymini2 用户对 private 下资源有读写权限
// daymini1 用户对 bucket 下资源有读写权限
// 用 daymini2 用户上传资源并赋予 aclprivate, 可以用 daymini1 用户使用 [带签名的URL] 下载下来
func Test_putFileWithACL(t *testing.T) {
	fileName := "daymini1_private3"
	putFileWithACL(dayMini2, oss.ACLPrivate, "private/"+fileName)
	getFile(dayMini1, "private/"+fileName, "/Users/daydaylw3/"+fileName)
}
