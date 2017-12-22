connect to SAMPLE;
DROP PROCEDURE BRANDLSPEED;

CREATE PROCEDURE BRANDLSPEED
SPECIFIC BRANDLSPEED
LANGUAGE SQL
BEGIN
  DECLARE timedf INTEGER DEFAULT 0;--
  DECLARE v_counter INTEGER DEFAULT 0;--
  DECLARE diff bigint;--
  DECLARE diff_d bigint;--
  DECLARE start_ts timestamp(6);--
  DECLARE end_ts timestamp(6);--
  DECLARE SQLSTATE CHAR(5);--

  DECLARE c1 CURSOR FOR SELECT IBMREQD FROM SYSIBM.SYSDUMMY1 FOR READ ONLY;--
  CALL DBMS_OUTPUT.PUT_LINE('Starting testspeed stored procedure : '|| start_ts);--


  WHILE v_counter < (100000) DO
    SET start_ts = CURRENT_TIMESTAMP;--
    OPEN c1;--
    SET end_ts = CURRENT_TIMESTAMP;--
    SET diff = TIMESTAMPDIFF(1,end_ts - start_ts);--
    SET timedf = timedf + diff;--
    SET v_counter = v_counter + 1;--
    CLOSE c1;--
  END WHILE;--

  SET diff_d = timedf/v_counter;--
  CALL DBMS_OUTPUT.PUT_LINE('Time used for open cursor: ' || timedf);--
  CALL DBMS_OUTPUT.PUT_LINE('Average Time n microseconds used for open cursor: ' || diff_d);--
  CALL DBMS_OUTPUT.PUT_LINE('Number of Cursor OPEN/CLOSE : ' || v_counter );--

END;
connect reset;
connect to SAMPLE;
SET SERVEROUTPUT ON;
CALL BRANDLSPEED;
terminate;

