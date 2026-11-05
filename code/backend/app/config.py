from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    DATABASE_URL: str = "postgresql://iot_admin:secure_password123@localhost:5432/infusion_iot"
    MQTT_BROKER_HOST: str = "localhost"
    MQTT_BROKER_PORT: int = 1883
    JWT_SECRET: str = "change-me-in-production"
    CORS_ORIGINS: str = "*"

    class Config:
        env_file = ".env"

settings = Settings()

