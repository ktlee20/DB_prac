USE Pokemon;

SELECT T.name from Trainer T, CatchedPokemon C WHERE T.id = C.owner_id GROUP BY T.name ORDER BY COUNT(*) DESC, T.name;

CREATE VIEW POKNUM AS SELECT COUNT(*) AS num FROM Pokemon GROUP BY type ORDER BY num DESC limit 2;

SELECT name FROM Pokemon AS p1, (SELECT t.type , COUNT(*) AS num FROM Pokemon t GROUP BY t.type ORDER BY num DESC) AS p2 WHERE p2.type = p1.type AND num IN (SELECT * FROM POKNUM) ORDER BY name;

DROP VIEW POKNUM;

SELECT name FROM Pokemon WHERE name LIKE "_o%";

SELECT nickname FROM CatchedPokemon WHERE level >= 50 ORDER BY nickname;

SELECT name FROM Pokemon WHERE 6 = CHAR_LENGTH(name) ORDER BY name;

SELECT name FROM Trainer WHERE hometown = "Blue City" ORDER BY name;

SELECT DISTINCT hometown FROM Trainer ORDER BY hometown;

SELECT AVG(level) from CatchedPokemon c, Trainer t WHERE c.owner_id = t.id AND t.name = 'Red';

SELECT nickname FROM CatchedPokemon WHERE nickname NOT LIKE "T%" ORDER BY nickname;

SELECT nickname FROM CatchedPokemon WHERE level <= 50 AND owner_id >= 6 ORDER BY nickname;

SELECT id, name FROM Pokemon ORDER BY id;

SELECT nickname FROM CatchedPokemon WHERE level <= 50 ORDER BY level;

SELECT p.name, p.id FROM Trainer t, Pokemon p, CatchedPokemon c WHERE t.id = c.owner_id AND p.id = c.pid AND t.hometown = "Sangnok City" ORDER BY p.id;

SELECT DISTINCT nickname FROM Gym g, CatchedPokemon c, Pokemon p WHERE c.pid = p.id AND g.leader_id = c.owner_id AND p.type = "Water" ORDER BY nickname;

SELECT COUNT(*) FROM Evolution;

SELECT COUNT(*) FROM Pokemon WHERE type = "Water" OR type = "Electric" OR type = "Psychic";

SELECT COUNT(*) FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id AND hometown = "Sangnok City";

SELECT MAX(level) FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id AND hometown = "Sangnok City";

SELECT COUNT(type) FROM Pokemon p, Gym g, CatchedPokemon c WHERE p.id = c.pid AND g.leader_id = c.owner_id AND City = "Sangnok City";

SELECT t.name, COUNT(*) AS num FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id AND t.hometown = "Sangnok City" GROUP BY t.name ORDER BY num;

SELECT name FROM Pokemon WHERE name LIKE "a%" OR name LIKE "e%" OR name LIKE "i%" OR name LIKE "o%" OR name LIKE "u%" ORDER BY name;

SELECT type, COUNT(*) AS num  FROM Pokemon GROUP BY type ORDER BY num, type;

SELECT DISTINCT t.name FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id AND level <= 10 ORDER BY t.name;

SELECT DISTINCT t.hometown, AVG(level) AS al FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id GROUP BY t.hometown ORDER BY al;


CREATE VIEW SANGNOKPOC AS SELECT DISTINCT p.name, t.hometown FROM Pokemon p, CatchedPokemon c, Trainer t WHERE c.owner_id = t.id AND c.pid = p.id AND hometown = "Sangnok City";

CREATE VIEW BROWNPOC AS SELECT DISTINCT p.name, t.hometown FROM Pokemon p, CatchedPokemon c, Trainer t WHERE c.owner_id = t.id AND c.pid = p.id AND hometown = "Brown City";

SELECT DISTINCT s.name FROM BROWNPOC b , SANGNOKPOC s WHERE s.name = b.name;

DROP VIEW SANGNOKPOC;
DROP VIEW BROWNPOC;

SELECT nickname FROM CatchedPokemon WHERE nickname LIKE "% %" ORDER BY nickname DESC;

SELECT t.name, COUNT(*) AS num FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id GROUP BY t.name HAVING num >= 4 ORDER BY t.name;


SELECT t.name, AVG(level) AS lev  FROM Trainer t, CatchedPokemon c, Pokemon p WHERE t.id = c.owner_id AND p.id = c.pid AND p.type IN ("Normal", "Electric") GROUP BY t.name ORDER BY lev;


SELECT e1.before_id, p1.name, e1.after_id, p2.name, e2.after_id, p3.name FROM Evolution e1, Evolution e2, Pokemon p1, Pokemon p2, Pokemon p3 WHERE p1.id = e1.before_id AND p2.id = e1.after_id AND p3.id = e2.after_id AND e1.after_id = e2.before_id ORDER BY e1.before_id;

SELECT name FROM Pokemon WHERE id >= 10 AND id < 100 ORDER BY name;

SELECT p.name FROM Pokemon p WHERE p.id NOT IN (SELECT pid FROM CatchedPokemon) ORDER BY p.name;

SELECT SUM(level) FROM CatchedPokemon c, Trainer t WHERE t.id = c.owner_id AND t.name = "Matis";

SELECT t.name FROM Gym g, Trainer t WHERE t.id = g.leader_id ORDER BY name;

SELECT type, (COUNT(*)/(SELECT COUNT(*) FROM Pokemon)) AS ratio FROM Pokemon p GROUP BY type ORDER BY ratio DESC LIMIT 1;

SELECT DISTINCT t.name FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id AND c.nickname LIKE "A%" ORDER BY name;

SELECT t.name, SUM(LEVEL) AS LEV FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id GROUP BY t.name ORDER BY LEV DESC LIMIT 1;

SELECT name FROM Pokemon p, Evolution e WHERE p.id = e.after_id AND p.id NOT IN (SELECT p1.id FROM Pokemon p1, Evolution e1, Evolution e2 WHERE p1.id = e2.after_id AND e1.after_id = e2.before_id) ORDER BY name;

SELECT t.name, COUNT(nickname) FROM Trainer t, CatchedPokemon c WHERE t.id = c.owner_id GROUP BY t.name HAVING COUNT(nickname) >= 2 ORDER BY name;

SELECT t.hometown, c.nickname FROM Trainer t, CatchedPokemon c, (SELECT t1.hometown, MAX(level) AS m FROM Trainer t1, CatchedPokemon c1 WHERE t1.id = c1.owner_id GROUP BY t1.hometown) AS tt WHERE t.id = c.owner_id AND t.hometown = tt.hometown AND c.level = m ORDER BY t.hometown;

