# Simodiri
Istruzioni per l'esecuzione del progetto:
1. Eseguire il web control per far partire la mappa e rviz
2. Aprire un terminale e recarsi nella cartella contente il progetto che è denominata con il nome "progetto"
3. Eseguire il comando 
   rosrun progetto progetto /base_scan /cmd_vel
5. Aprire un altro terminale con cui si invieranno i comandi di velocità al robot, tramite il comando
   rostopic pub /cmd_vel /geometry_msgs/Twist <message>



