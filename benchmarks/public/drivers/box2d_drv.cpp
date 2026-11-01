#include "box2d/box2d.h"
#include <cstdio>
#include <vector>
// Pyramid scene in the style of Box2D's Tumbler/Pyramid testbed scenes.
int main(){ b2Vec2 g(0,-10); b2World w(g); b2BodyDef bd; b2Body* gr=w.CreateBody(&bd); b2EdgeShape e; e.SetTwoSided(b2Vec2(-40,0),b2Vec2(40,0)); gr->CreateFixture(&e,0);
 b2PolygonShape s; s.SetAsBox(0.5f,0.5f); b2Vec2 x(-7,0.75f),dx(0.5625f,1.25f),dy(1.125f,0); const int N=30; std::vector<b2Body*>* unused=0; b2Body* last=0;
 for(int i=0;i<N;i++){ b2Vec2 y=x; for(int j=i;j<N;j++){ b2BodyDef d; d.type=b2_dynamicBody; d.position=y; b2Body* b=w.CreateBody(&d); b->CreateFixture(&s,5); y+=dy; last=b;} x+=dx;}
 double sum=0; for(int t=0;t<1200;t++){ w.Step(1.f/60,8,3); }
 for(b2Body* b=w.GetBodyList();b;b=b->GetNext()){ b2Vec2 p=b->GetPosition(); sum+=p.x*0.5+p.y; }
 printf("%.6f %d %.6f %.6f\n",sum,w.GetContactCount(),last->GetPosition().x,last->GetPosition().y); }
