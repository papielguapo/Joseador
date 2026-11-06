#pragma once
struct ConnectivityState {
 enum Change {none,lost,restored};bool offline=false;
 Change update(int network,int internet){
  if(network<0&&internet<0)return none;
  const bool next=network==0||internet==0;
  if(next==offline)return none;
  offline=next;return offline?lost:restored;
 }
};
