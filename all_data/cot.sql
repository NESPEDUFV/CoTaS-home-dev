CREATE TABLE IF NOT EXISTS objeto(
    `id` INTEGER PRIMARY KEY,
    `ip` INTEGER,
    -- para simplificação aqui temos um json 
    -- em que o primeiro campo ("keys") são as chaves
    -- existentes dos contextos 
    `dados` TEXT 
);

INSERT INTO objeto (id, ip, dados) VALUES
(-3, 1010, "{teste do banco: 0}");