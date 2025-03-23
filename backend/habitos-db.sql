
CREATE TABLE tb_habitos (
    id SERIAL PRIMARY KEY,
    date_tracker DATE NOT NULL,
    contador_refeicao INT NOT NULL,
    contador_agua INT NOT NULL,
    contador_exercicio INT NOT NULL,
    contador_pomodoro INT NOT NULL
);

INSERT INTO tb_habitos (date_tracker, contador_refeicao, contador_agua, contador_exercicio, contador_pomodoro)
VALUES ('2025-03-23', 1, 2, 1, 2);