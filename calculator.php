<html><?php
// date_sunset(time(), SUNFUNCS_RET_STRING, Lat, Long, Zenith, TZ);
/* Lat: + N, - S
   Long: + E, - W
   Zenith: 90 sunset, 96 civil, 102 naut., 108 astro. twilight
   TZ: 0 = GMT  */
$full=isset($_GET['f']);
$opt=isset($_GET['opt']);
$tz=(int)$_GET['tz'];
$lat=(float)$_GET['lat'];
$long=(float)$_GET['long'];
if(isset($_GET['y'])){
  $year=(int)$_GET['y'];
  if($year==0) $year=date("Y",time());
  echo '<title>'.$year.'</title><body style="font: 13pt segoe;">';
  $date=strtotime("1-1-".$year);
  for($i=1;$i<=12;$i++){
    $stopflag=false;
    for($j=1;$j<=31;$j++){
      $number=cal_days_in_month(CAL_GREGORIAN, date("n",$date), $year);
      $off_time=date_sunset($date, SUNFUNCS_RET_STRING, $lat, $long, 96, $tz);
      $on_time=date_sunrise($date, SUNFUNCS_RET_STRING, $lat, $long, 90, $tz);
      if($full && !$stopflag) echo date("d/m/Y",$date).' - '.$on_time.' - '.$off_time.'<br>';
      $on_hr[$i][$j]=substr($on_time,0,2);
      $on_mn[$i][$j]=substr($on_time,3,2);
      $off_hr[$i][$j]=substr($off_time,0,2);
      $off_mn[$i][$j]=substr($off_time,3,2);
      if($number==date("d",$date)){
        $stopflag=true;
        $date+=24*60*60;
      }
      if(!$stopflag) $date+=24*60*60;
    }
  }
  if($full) echo '<br>';
   else{
    for($k=1;$k<=4;$k++){
      $optim=array();
      $optflag=(!($k==2 || $k==4) && $opt);
      if(!$optflag) echo 'const uint8_t ';
      switch($k){
        case 1: if(!$optflag) echo 'dawn_hr'; $link=&$on_hr;  break;
        case 2: if(!$optflag) echo 'dawn_mn'; $link=&$on_mn;  break;
        case 3: if(!$optflag) echo 'dusk_hr'; $link=&$off_hr; break;
        case 4: if(!$optflag) echo 'dusk_mn'; $link=&$off_mn; break;
      }
      if(!$optflag) echo '[12][31] PROGMEM={<br>';
      for($i=1;$i<=12;$i++){
        if(!$optflag) echo '&nbsp; {';
        for($j=1;$j<=31;$j++){
          if($k==1 || $k==3) $optim[]=$link[$i][$j];
          if(!$optflag){
            if(!isset($_GET['x'])){
              if(substr($link[$i][$j],0,1)=='0') echo ' '.substr($link[$i][$j],1,1);
               else echo $link[$i][$j];
            }else echo '0x'.str_pad(strtoupper(dechex((int)$link[$i][$j])),2,'0',STR_PAD_LEFT);
            if($j<31) echo ',';
          }
        }
        if(!$optflag){
          echo '}';
          if($i<12) echo ',';
          echo '<br>';
        }
      }
      if(!$optflag) echo '};<br>';
      if($k==1 || $k==3){
        $optim1=array_unique($optim);
        $optim2=array_unique(array_reverse($optim));
        $index1=array_keys($optim1);
        $index2=array_keys($optim2);
        for($i=0;$i<count($optim1);$i++){
          $s[$i]='if((';
          if($i!=0){
            $s[$i].='nday=>';
            $s[$i].=array_values($index1)[$i];
            if($i!=count($optim1)-1) $s[$i].=' && ';
          }
          if($i+1!=count($optim1)){
            $s[$i].='nday<=';
            $s[$i].=array_values($index1)[$i+1]-1;
          }
          $s[$i].=')';
        }
        for($i=count($optim2);$i>0;$i--){
          $s[$i-1].=' && (';
          if($i!=count($optim2)){
            $s[$i-1].='nday>=';
            $s[$i-1].=372-array_values($index2)[$i];
            if($i!=1) $s[$i-1].=' && ';
          }
          if($i!=1){
            $s[$i-1].='nday<=';
            $s[$i-1].=372-array_values($index2)[$i-1]-1;
          }
          $s[$i-1].=')) dawn_hr=';
          if(substr(array_values($optim1)[$i-1],0,1)=='0') $s[$i-1].=substr(array_values($optim1)[$i-1],1,1);
           else $s[$i-1].=array_values($optim1)[$i-1];
          $s[$i-1].=';';
          if ($optflag) echo ''.$s[$i-1].'<br>';
        }
      }
      unset($optim);
    }
    echo '<br><a href='.$_SERVER['PHP_SELF'].'?y='.$year.'&tz='.$tz.'&lat='.$lat.'&long='.$long.'&f>Show full calendar</a> ';
    echo '<a href='.$_SERVER['PHP_SELF'].'?y='.$year.'&tz='.$tz.'&lat='.$lat.'&long='.$long;
    if(!$opt) echo '&opt>Hardcode hours</a> ';
     else echo '>Array hours</a> ';
    echo '<a href='.$_SERVER['PHP_SELF'].'?y='.$year.'&tz='.$tz.'&lat='.$lat.'&long='.$long;
    if(!isset($_GET['x'])) echo '&x>HEX</a>';
     else echo '>DEC</a>';
  }
  echo ' <a href='.$_SERVER['PHP_SELF'].'>Reset</a> <a href='.$_SERVER['PHP_SELF'].'?help>Help</a>';
}else if(isset($_GET['help'])){
  echo '<title>How to use</title><body>Header:<pre>#include &lt;avr/pgmspace.h&gt;</pre>Read:
   <pre>byte x=pgm_read_byte(&(dusk_hr[i][j]));</pre>Optimized hour calculation code should be inside <b>loop()</b>.
   <br><br><a href='.$_SERVER['PHP_SELF'].'>Reset</a>';
}else{
 echo '<title>Twilight data calculator</title><body><form action='.$_SERVER['PHP_SELF'].' method=get>
 Year: <input type=text size=5 name=y value='.date("Y").'>
 Lat: <input type=text size=7 name=lat placeholder="N +, S -">
 Long: <input type=text size=7 name=long placeholder="E +, W -">
 TZone: <input type=text size=3 name=tz placeholder="0 = GMT">
 <input type=submit value=">>>"></form><a href=http://www.iplocation.net/>iplocation.net/</a>';
}
?></body></html>
