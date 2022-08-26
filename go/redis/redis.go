package redis

import (
	"context"
	"fmt"

	"github.com/go-redis/redis/v8"
)

func T() {
	c := redis.NewUniversalClient(&redis.UniversalOptions{
		Addrs: []string{
			"112.74.174.20:6379",
		},
		DB:       0,
		Password: "abc123",
	})
	_, err := c.Ping(context.Background()).Result()
	if err != nil {
		fmt.Println(err)
	}
	ctx := context.Background()
	pipeline := c.Pipeline()
	pipeline.SAdd(ctx, "set1", "a", "b", "c", "d")
	pipeline.SAdd(ctx, "set2", "c", "d", "e", "f", "g")
	pipeline.SAdd(ctx, "set3", "g", "h", "i")
	pipeline.SUnionStore(ctx, "settmp", "set1", "set2", "set3")
	pipeline.SCard(ctx, "settmp")
	pipeline.Del(ctx, "settmp")
	exec, err := pipeline.Exec(ctx)
	fmt.Println(exec[5].String())
	fmt.Println(len(exec), exec, err)
}
