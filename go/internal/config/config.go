package config

import (
	"strings"

	"github.com/spf13/viper"
)

type Config struct {
	Server    ServerConfig    `mapstructure:"server"`
	Mqtt      MqttConfig      `mapstructure:"mqtt"`
	Database  DatabaseConfig  `mapstructure:"database"`
	Processor ProcessorConfig `mapstructure:"processor"`
}

type ServerConfig struct {
	Address string `mapstructure:"address"`
}

type MqttConfig struct {
	Broker   string `mapstructure:"broker"`
	ClientID string `mapstructure:"client_id"`
	Topic    string `mapstructure:"topic"`
}

type DatabaseConfig struct {
	Filepath string `mapstructure:"filepath"`
}

type ProcessorConfig struct {
	NumWorkers int `mapstructure:"num_workers"`
	QueueSize  int `mapstructure:"queue_size"`
}

func LoadConfig(path string) (*Config, error) {
	viper.AddConfigPath(path)
	viper.SetConfigName("config")
	viper.SetConfigType("yml")

	viper.AutomaticEnv()
	viper.SetEnvPrefix("V")
	viper.SetEnvKeyReplacer(strings.NewReplacer(".", "_"))

	if err := viper.ReadInConfig(); err != nil {
		return nil, err
	}

	var config Config
	if err := viper.Unmarshal(&config); err != nil {
		return nil, err
	}

	return &config, nil
}
