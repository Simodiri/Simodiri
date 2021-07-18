
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "tf/transform_listener.h"
#include "geometry_utils_fd.h"
#include "sensor_msgs/PointCloud.h"
#include "tf/message_filter.h"
#include "message_filters/subscriber.h"
#include "laser_geometry/laser_geometry.h"
#include <Eigen/StdVector>
#include <Eigen/Geometry>
#include <sstream>
ros::Publisher vel_pub;
float mod_vel_x=0;
float mod_vel_y=0;
float angular=0;
bool vel_mod=false;
float target_x=0;
using namespace Eigen;

void VelModCallBack(const geometry_msgs::Twist::ConstPtr& msg){
 ROS_INFO("Ricevo le coordinate x:%f y:%f z:%f",msg->linear.x,msg->linear.y,msg->angular.z);
vel_mod=true; //è arrivato un comando
 mod_vel_x=msg->linear.x;
 mod_vel_y=msg->linear.y;
 angular=msg->angular.z;

 target_x=mod_vel_x*0.1;
 //target_y=vel_y*0.2;
}
void LaserCallBack_aux(const sensor_msgs::LaserScan::ConstPtr& scan_in){
   //devo estrarre i punti dal Laser_scan
   laser_geometry::LaserProjection projector_;
   sensor_msgs::PointCloud cloud;
   tf::TransformListener listener_;
   tf::StampedTransform transform;
try
     {
         projector_.transformLaserScanToPointCloud(
            "base_laser_link",*scan_in, cloud,listener_);
      }
     catch (tf::TransformException& e)
      {
          ROS_ERROR("%s",e.what());
          return;
     }
       Eigen::Isometry2f transform_laser = convertPose2D(transform);
  // Extract points from raw laser scan and paint them on canvas
       Eigen::Vector2f p;
       float sum_fx=0; //somma forze su x
       float sum_fy=0; //somma forze su y
       for(auto& point :cloud.points){
         p(0)=point.x;
         p(1)=point.y;
         
         p=transform_laser*p; //ottengo il punto trasformato
         float modulo=1/sqrt(point.x*point.x+point.y*point.y);
       
         sum_fx+=p(0)*modulo*modulo; //coordinata x normalizzata
         sum_fy+=p(1)*modulo*modulo;
       }
       //prendiamo l'opposto delle forze applicate perchè hanno stessa direzioni ma verso opposto dei robot e ostacolo
       sum_fx=-sum_fx;
       sum_fy=-sum_fy;
        geometry_msgs::Twist send;
       float distanza=sqrt(target_x*target_x+0*0); // è la distanza dal punto in cui voglio far ruotare il robot
       ROS_INFO("La distanza è %f \n", distanza);
       
       send.angular.z=distanza*sum_fy/30;
       //send.angular.z=distanza;
        //agisco sulle componenti x ed y della velocità
        sum_fx*=abs(mod_vel_x)/600;
        //sum_fy*=abs(mod_vel_y)/500;
       // send.angular.z=distanza;
       ROS_INFO("Le forze sono %f %f %f\n", sum_fx,sum_fy);
       if(sum_fx<0){
           send.linear.x= sum_fx + mod_vel_x;

       }else{
         send.linear.x=- sum_fx + mod_vel_x;
       }
              
       //if(send.linear.x>0 && send.linear.x<0.1 || sum_fx>1){
       // send.angular.z=1;
      //}else{
        send.angular.z+=angular;
        // }
       
       send.linear.y=mod_vel_y+sum_fy;
      
       
       ROS_INFO("Nuovo vettore velocità %f %f %f\n", send.linear.x,send.linear.y,send.angular.z);
          
      vel_pub.publish(send);

}
void LaserCallBack(const sensor_msgs::LaserScan::ConstPtr& scan_in){
  if(vel_mod) LaserCallBack_aux(scan_in);
  return;
}
int main(int argc, char **argv){
   if(argc<2){
       ROS_INFO("Errore:  numero di parametri errato");
       return -1;
   }  
     ROS_INFO("Inserire come parametri: 1. Topic per scan 2. Topic per cmd_vel");
   ros::init(argc,argv,"progetto");

   ros::NodeHandle n;
    ros::Rate loop_rate(10);
   vel_pub=n.advertise<geometry_msgs::Twist>(argv[2],1000);//publisher per pubblicare i comandi di velocità modificati
   ROS_INFO("Ho avviato il publisher su %s",argv[2]);
   
   ros::Subscriber base_sub=n.subscribe(argv[1],1000,LaserCallBack);// subscriber per ricevere i comandi dal laser scan
   ROS_INFO("Ho avviato il subscriber su %s",argv[1]);
   
   ros::Subscriber vel_sub=n.subscribe("/cmd_vel_mod",1000,VelModCallBack);// subscriber per il comando di velocità modificato
   ROS_INFO("Ho avviato il subscriber su /cmd_vel_mod") ;
    ROS_INFO("ciao") ;

   ROS_INFO("Inviare al topic /cmd_vel_mod il comando per far muovere il robot");
   ros::spin();
   
  return 0;


}