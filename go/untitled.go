package main

import (
	"fmt"
)

// 下面这个栗子将一个切片中的所有元素循环移了一位
func main() {
	forrange()
	for1()
	for2()
}

// [3 4 5 7 2]
func forrange() {
	x := []int {2, 3, 4, 5, 7}
	t := x[0]
	var i int
	for i, x[i] = range x {}
	x[i] = t
	fmt.Println(x)
}

// [3 4 5 7 2]
func for1() {
	x := []int {2, 3, 4, 5, 7}
	t := x[0]
	var i int
	for index := 0; index < len(x); index++ {
		x[i] = x[index]
		i = index
	}
	x[i] = t
	fmt.Println(x)
}

// [2 3 4 5 2]
func for2() {
	x := []int {2, 3, 4, 5, 7}
	t := x[0]
	var i int
	for index := 0; index < len(x); index++ {
		i = index
		x[i] = x[index]
	}
	x[i] = t
	fmt.Println(x)
}