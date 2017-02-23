###########################################################################################
###########################################################################################
## VELO-METER POST-PROCESSING ALGORTIHM * CIVL440 * Simone Tengattini * Updated 20NOV16 ***
#******************************************************************************************

## READ in DATA
setwd("C:/Users/tengas/Google Drive/University/UBC/2016 Winter Term1/CIVL440/Project/Bike_Speeds/")

fn <- list.files(path = "C:/Users/tengas/Google Drive/University/UBC/2016 Winter Term1/CIVL440/Project/Bike_Speeds/BikeSpeedRawData/")
stats_bikes_matrix <- as.data.frame(matrix(NA,length(fn),8))
Skim_data_tot <- list()

## mode fucniton for statistical analysis
Mode <- function(x) {
  ux <- unique(x)
  ux[which.max(tabulate(match(x, ux)))]
}


for (j in 1:length(fn)) 
{

TimeStamps_survey <- list()
TimeStamps_survey <- read.csv(paste0("C:/Users/tengas/Google Drive/University/UBC/2016 Winter Term1/CIVL440/Project/Bike_Speeds/BikeSpeedRawData/",fn[j]), header = T, skip = 1)
TimeStamps_survey <- TimeStamps_survey[-c((nrow(TimeStamps_survey)-1),nrow(TimeStamps_survey)),]
TimeStamps_survey$TIME_ms <- as.numeric(levels(TimeStamps_survey$TIME_ms)[TimeStamps_survey$TIME_ms])
TimeStamps_survey$TIME_ms <- TimeStamps_survey$TIME_ms-TimeStamps_survey$TIME_ms[1]
TimeStamps_survey$X.BeamBroken <- as.numeric(levels(TimeStamps_survey$X.BeamBroken)[TimeStamps_survey$X.BeamBroken])

## DATA manipulation
sp_mean <- NA
sp_sd <- NA
sp_cnt <- 1
s_min <- 300
s_max <- 300
for (s in s_min:s_max)
{
spacing = s

for (i in 2:nrow(TimeStamps_survey))
{
  TimeStamps_survey$TimeRel[i] <- (TimeStamps_survey$TIME_ms[i]-TimeStamps_survey$TIME_ms[i-1])
  TimeStamps_survey$BikeFlag[i] <-NA
  TimeStamps_survey$BikeSpeed[i] <-NA
  TimeStamps_survey$WheelBase[i] <-NA
  TimeStamps_survey$HeadWay[i] <-NA
  TimeStamps_survey$BikeFlagCum[i] <- 0
}




#initialize for first bike
  TimeStamps_survey$BikeFlag[1:4] <- 1
  TimeStamps_survey$BikeFlagCum[1:4] <- 1:4




bike=1
for (i in 5:nrow(TimeStamps_survey))
{ 
  if (i!=nrow(TimeStamps_survey))
  {
    if (TimeStamps_survey$TimeRel[i]>(0.9*sum(TimeStamps_survey$TimeRel[(i-2):(i-1)])))
    {
      bike=bike+1
      TimeStamps_survey$BikeFlag[i] <- bike

      #new bike, get prebious bike cinematic infos
      TimeStamps_survey$BikeSpeed[i-1] <- mean(c(spacing/(TimeStamps_survey$TimeRel[i-1]),(spacing/(TimeStamps_survey$TimeRel[i-3]))))
      TimeStamps_survey$WheelBase[i-1] <- TimeStamps_survey$BikeSpeed[i-1]*sum(TimeStamps_survey$TimeRel[(i-2):(i-1)])/1000
      TimeStamps_survey$HeadWay[i-1] <- (TimeStamps_survey$TIME_ms[i]-TimeStamps_survey$TIME_ms[i-4])/1000
      TimeStamps_survey$BikeFlagCum[i]=1
    }
    else
    {
      TimeStamps_survey$BikeFlag[i] <- bike
      
      if (TimeStamps_survey$BikeFlag[i]==TimeStamps_survey$BikeFlag[i-1])
      { 
      TimeStamps_survey$BikeFlagCum[i] <- TimeStamps_survey$BikeFlagCum[i-1]+1
      }
    }
  }
  else 
  {
    #new bike, get cinematic infos for last bike
    TimeStamps_survey$BikeFlag[i] <- bike
    TimeStamps_survey$BikeSpeed[i] <- mean(c(spacing/(TimeStamps_survey$TimeRel[i]),(spacing/(TimeStamps_survey$TimeRel[i-2]))))
    TimeStamps_survey$WheelBase[i] <- TimeStamps_survey$BikeSpeed[i]*sum(TimeStamps_survey$TimeRel[(i-1):(i)])/1000 
    TimeStamps_survey$BikeFlagCum[i]=TimeStamps_survey$BikeFlagCum[i-1]+1
  }
}

## TAKE ONLY WHAT IS NEEDED from "timestaps_survey

#skim so that they have >2 wheels
Skim_data <- TimeStamps_survey[which(TimeStamps_survey$BikeFlagCum>=4),c("BikeFlag","BikeSpeed","WheelBase","HeadWay","BikeFlagCum")]

#skim so that there are not crazy speeds
Skim_data <- Skim_data[which(Skim_data$BikeSpeed<15),]
Skim_data <- Skim_data[which(Skim_data$BikeSpeed>1),]

#Skim so that wheelbase is reasonable
Skim_data <- Skim_data[which(Skim_data$WheelBase<1.75),]
Skim_data <- Skim_data[which(Skim_data$WheelBase>0.5),]

Skim_data_tot[[j]] <- Skim_data

mean <- apply(Skim_data,2,mean, na.rm = T)
sd <- apply(Skim_data,2,sd, na.rm = T)
sp_mean[sp_cnt] <- mean(Skim_data$BikeSpeed,na.rm = T)
sp_sd[sp_cnt] <- sd(Skim_data$BikeSpeed, na.rm = T)
sp_cnt=sp_cnt+1

}

plot(s_min:s_max,sp_mean, xlab = "Laser Spacing [mm]", ylab = "Sample Mean Speed [m/s]", main = "Sensitivity spacing - mean speed")
plot(s_min:s_max,sp_sd, xlab = "Laser Spacing [mm]", ylab = "Sample Mean St.Dev. [m/s]",main = "Sensitivity spacing - st dev.speed")

hist(Skim_data$HeadWay, breaks = 50)

## Get fundamentla diagram
#Skim_data$AvgFlow <- 1/Skim_data$HeadWay*3600
#Skim_data$AvgDens <- Skim_data$AvgFlow/Skim_data$BikeSpeed
#Skim_data <- Skim_data[which(Skim_data$AvgDens<200),]
#plot(Skim_data$AvgFlow[-nrow(Skim_data)], (Skim_data$BikeSpeed[-nrow(Skim_data)])*3.6)
#plot(Skim_data$AvgDens[-nrow(Skim_data)], (Skim_data$BikeSpeed[-nrow(Skim_data)])*3.6)




#print stats
Sample_size <- dim(Skim_data)[1]
wheelbase_avg <- mean(Skim_data$WheelBase)
wheelbase_sd <- sd(Skim_data$WheelBase)
hdwy_mode <- Mode(Skim_data$HeadWay)
hdwy_med <- median(Skim_data$HeadWay, na.rm = T)
hdwy_mean <- mean(Skim_data$HeadWay, na.rm = T)

stats_bikes <- c(sp_mean,sp_sd,wheelbase_avg,wheelbase_sd,Sample_size,hdwy_mode,hdwy_med,hdwy_mean)
stats_bikes_matrix[j,] <- stats_bikes
}

colnames(stats_bikes_matrix) <- c("Mean Speed","St Dev Speed","wheelbase mean","wheelbase sd","Sample Size","Mode Headway","median hdwy","mean hdwy")
rownames(stats_bikes_matrix) <- fn
write.csv(stats_bikes_matrix,"CIVL440_Speed_bike_stats_RAW_11_20.csv")


##################################################################################
## END * END * END * END * END * END *END * END * END * END * END * END * END  ##
#################################################################################