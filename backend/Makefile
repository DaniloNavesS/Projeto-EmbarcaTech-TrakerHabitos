.PHONY: up down restart logs exec-backend exec-db

up:
	docker compose up -d --build

down:
	docker compose down

restart: down up

logs:
	docker compose logs -f

exec-backend:
	docker compose exec backend sh

exec-db:
	docker compose exec db sh
