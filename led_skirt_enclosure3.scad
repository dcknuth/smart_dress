// Arduino Nano compatible enclosure and 2x 18650s
//34567891123456789212345678931234567894123456789512345678961234567897123456789

// Global settings
$fn = 100;
FUDGE = 0.01; // To ensure non-perfect difference edges
printCover = false;
extra = 10.0; // Some extra space

// Specs from arduino.cc which matched the measured size well
boardWidth = 18.0;
boardLength = 45.0;
// Measured specs
boardHeight = 7.5; // lowest reverse-side to top of USB
pcbThickness = 1.8;
batHeight = 22.0;
batWidth = 40.1;
batLength = 78.4;
batXoff = 8.4;  // Room from the edge plus the series wire on back
batYoff = -0.6;    // Close to what will be the lower edge
clipWidth = 5;
// Options
tol = 0.4; // room to make sure things fit
// Box options
botRoom = 2.5; // bottom side components
wallThickness = 3.0;
cornerR = 4.0; // radius for rounding box corners
hasWireChannelsY = false;
hasWireChannelsX = false;
wireChannelD = 6;
hasVents = true;
numVents = 4;
ventWidth = 3; // up to you to make sure this is not too many
usePins = true; // for microphone
pinH = 3;
pinR = 2.9/2;  // Slightly less than the hole diameter
pinLipH = 1;
pinLipT = 0.3;
lidHeight = wallThickness;
// Mount options
mountDia = 5.5;
mountHeight = botRoom;
screwDia = 2.9;
threadPitch = .448;
// Microphone board info
micD = 10.5;
micBoardW = 14.0;
micBoardL = 25.5;
micMountT = 2.0;
micMountD = 2.5;
micFromTop = 2.0;
micY = micBoardW/2+0.5;
// Wire connector info
wireConnectD = 15.6;
// Knob/button info
knobD = 7.5;
knobBase = 13.0;
knobY = 40.5;
baseDepth = 1.5;
baseCutX = 2.2;
baseCutY = 1.5;
// Nano holder info
nanoLift = 4.5;
nanoBaseX = 4.0;
nanoHoldA = 4.0;
nanoHoldB = boardWidth+1.5;
// Power switch info
switchX = 20.15;
switchY = 12.55;
switchXOff = nanoBaseX+batLength+12;
switchZOff = 4.0;

module Loop(x=6, y=12, z=12) {
  difference() {
    hull(){
      translate([-(x-2)/2,-(y-2)/2,1])
        sphere(1);
      translate([-(x-2)/2,(y-2)/2,1])
        sphere(1);
      translate([(x-2)/2,(y-2)/2,1])
        sphere(1);
      translate([(x-2)/2,-(y-2)/2,1])
        sphere(1);
      translate([-(x-2)/2,-(y-2)/2,z-1])
        sphere(1);
      translate([-(x-2)/2,(y-2)/2,z-1])
        sphere(1);
      translate([(x-2)/2,(y-2)/2,z-1])
        sphere(1);
      translate([(x-2)/2,-(y-2)/2,z-1])
        sphere(1);
    }
    translate([0,0,6])
      rotate([0,90,0])
        cylinder(h=x+FUDGE, d=7, center=true);
  }
}

module NanoChannel(bx, by, baseD, notchD, holdD=1.5) {
  translate([-bx/2,-bx/2,0])
    union() {
      difference() {
        cube([bx,bx,baseD+notchD]);
        translate([bx/4,-FUDGE,baseD])
          cube([bx/2+FUDGE,bx/2+FUDGE,notchD+FUDGE]);
      }
      if(holdD != 0) {
        translate([bx/2,bx/1.9,baseD+notchD-holdD/2.5])
          sphere(d=holdD);
      }
    }
}

module knobB(knobD, x, cutX, cutY, z) {
  union() {
    difference() {
      cube([x,x,z], center=true);
      translate([0,x/2-cutY/2,0])
        cube([cutX+FUDGE,cutY+FUDGE,z+FUDGE], center=true);
      cylinder(h=z+FUDGE, d=knobD, center=true);
    }
    translate([x/2,-(x/2-FUDGE),z/2])
      rotate([180,90,0])
        linear_extrude(height=x)
          polygon(points=[[0,0],[z,0],[0,z],[0,0]]);
  }
}

module micMount(D, x, z, md) {
  union() {
    difference() {
      cube([x,x,z], center=true);
      cylinder(h=z+FUDGE, d=D, center=true);
      translate([x/2.7,x/2.9,0])
        cylinder(h=z+FUDGE, d=md, center=true);
      translate([-x/2.7,x/2.9,0])
        cylinder(h=z+FUDGE, d=md, center=true);
    }
    translate([x/2,-(x/2-FUDGE),z/2])
      rotate([180,90,0])
        linear_extrude(height=x)
          polygon(points=[[0,0],[z,0],[0,z],[0,0]]);
  }
}

module RoundedBox(l, w, h, t, r) {
    // Rounded box with open top
    difference() {
        hull() {
            cylinder(h=h, r=r, center=false);
            translate([0,w,0])
                cylinder(h=h, r=r, center=false);
            translate([l,w,0])
                cylinder(h=h, r=r, center=false);
            translate([l,0,0])
                cylinder(h=h, r=r, center=false);
        }
        hull() {
            translate([0,0,t])
                cylinder(h=h, r=r-t, center=false);
            translate([0,w,t])
                cylinder(h=h, r=r-t, center=false);
            translate([l,w,t])
                cylinder(h=h, r=r-t, center=false);
            translate([l,0,t])
                cylinder(h=h, r=r-t, center=false);
        }
    }
}

module MountPoint(d, h, ss, p) {
    // ss = screw outside diameter. If non-zero, put threads in the top
    // p = thread pitch
    if(usePins) {
        union() {
            cylinder(h=h, d=d);
            translate([0,0,h-FUDGE/2])
                pin(h=pinH, r=pinR, lh=pinLipH, lt=pinLipT, side=false);
        }
    } else {
        difference() {
            cylinder(h=h, d=d);
            translate([0,0,FUDGE])
                ScrewHole(ss, h, pitch=p);
        }
    }
}

module Tab(width) {
    rotate([90,0,90])
        linear_extrude(height=width, twist = 0)
            polygon([
                [0,0],
                [width/2,0],
                [width/2,width/2],
                [width/2+width/8,width],
                [width/2,width+width/2],
                [width/2,width+width/2+width/4],
                [0,width+width/2+width/4],
                [0,0]]);
}

module Clip(width,clipStart,height) {
    rotate([90,0,90])
        linear_extrude(height=width, twist = 0)
            polygon([
                [0,0],
                [width/2,0],
                [width/2,clipStart],
                [width/2+width/8,clipStart+(height-clipStart)/2],
                [width/2,height],
                [0,height],
                [0,0]]);
}

module batHold(bx, baseD, notchD, tw) {
  // Four corner holds
  translate([batXoff,batYoff,wallThickness])
    rotate([0,0,0])
      cornerHold(bx=3, baseD=1, notchD=2.5, holdD=0);
}

module Lid(l, w, h, r, t=0) {
  union() {
    hull() {  // slight overhang to help get cover off
            cylinder(h=h, r1=r, r2=r*1.3, center=false);
            translate([0,w,0])
                cylinder(h=h, r1=r, r2=r*1.3, center=false);
            translate([l,w,0])
                cylinder(h=h, r1=r, r2=r*1.3, center=false);
            translate([l,0,0])
                cylinder(h=h, r1=r, r2=r*1.3, center=false); 
    }
    translate([l/2-l/4.5,t/2-(cornerR-wallThickness), h-FUDGE/2])
      rotate([0,0,180])
        Tab(t);
    translate([l/2+l/4.5+t/2,t/2-(cornerR-wallThickness), h-FUDGE/2])
      rotate([0,0,180])
        Tab(t);
    translate([l/2-l/4.5, (boxWidth-t/2)+(cornerR-wallThickness), h-FUDGE/2])
      Tab(t);
    translate([l/2+l/4.5, (boxWidth-t/2)+(cornerR-wallThickness), h-FUDGE/2])
      Tab(t);
  }
}
            
boxHeight = batHeight+botRoom+2+2*wallThickness+tol;
boxWidth = boardLength+extra/4+tol; // to cylinder centers (-2*r)
boxLength = batLength+boardWidth+extra+8+tol;

if(!printCover) {
  union() {
    difference() {
      RoundedBox(l=boxLength, w=boxWidth, h=boxHeight,
          t=wallThickness, r=cornerR);
      if(hasWireChannelsY) { // wire routing holes
        translate([boxLength/2,-(cornerR+FUDGE/2),boxHeight])
          rotate([-90,0,0]) // sticking out in positive Y
            cylinder(h=wallThickness+FUDGE, d=wireChannelD,
                center=false);
        translate([boxLength/2,boxWidth+cornerR+FUDGE/2,boxHeight])
          rotate([90,0,0]) // sticking out in negative Y
            cylinder(h=wallThickness+FUDGE, d=wireChannelD,
                  center=false);
      }
      if(hasWireChannelsX) { // wire routing holes
        translate([-(cornerR+FUDGE/2),boxWidth/2,boxHeight])
          rotate([0,90,0]) // sticking out in positive x
            cylinder(h=wallThickness+FUDGE, d=wireChannelD,
                  center=false);
        translate([boxLength+cornerR+FUDGE/2,boxWidth/2,boxHeight])
          rotate([0,-90,0]) // sticking out in negative Y
            cylinder(h=wallThickness+FUDGE, d=wireChannelD,
                  center=false);
      }
      if(hasVents) {
        l = 0.68*boxLength; // venting 65% of box length
        for (i=[0:numVents-1]) {
          translate([boxLength*(0.25/2)+i*(l/numVents)+(l/numVents)/2,
                -(cornerR-(wallThickness+FUDGE/2)/2),
                boxHeight/2])
            cube([ventWidth,wallThickness+FUDGE,boxHeight*0.45],
                center=true);
        }
        for (i=[0:numVents-1]) {
          translate([boxLength*(0.25/2)+i*(l/numVents)+(l/numVents)/2,
                  boxWidth+(cornerR-(wallThickness+FUDGE/2)/2),
                  boxHeight/2])
            cube([ventWidth,wallThickness+FUDGE,boxHeight*0.45],
                center=true);
        }
      }
      // Hole for microphone
      translate([boxLength+wallThickness/2+cornerR-wallThickness,
          micY,boxHeight-(micD/2+micFromTop+1)])
        rotate([0,90,0])
          cylinder(h=wallThickness+FUDGE, d=micD, center=true);
      // Hole for wire connector
      translate([boxLength+wallThickness/2+cornerR-wallThickness,
          boxWidth/2.0,12.5])
        rotate([0,90,0])
          cylinder(h=wallThickness+FUDGE, d=wireConnectD, center=true);
      // Hole for knob/button
      translate([boxLength+wallThickness/2+cornerR-wallThickness,
          knobY,boxHeight/1.5])
        rotate([0,90,0])
          cylinder(h=wallThickness+FUDGE, d=knobD, center=true);
      // on/off switch
      translate([switchXOff,boxWidth+wallThickness/2+1, boxHeight/2])
        rotate([-90,90,0])
          difference() {
            cube([switchX,switchY,wallThickness+FUDGE], center=true);
            translate([-switchX/2,0,wallThickness/3.6])
              rotate([90,0,0])
                cylinder(h=switchY/2, d=0.8, center=true);
            translate([switchX/2,0,wallThickness/3.6])
              rotate([90,0,0])
                cylinder(h=switchY/2, d=0.8, center=true);
          }
      // lid subtract to get the indentations
      translate([0, boxWidth, boxHeight+lidHeight])
        rotate([180,0,0])
          Lid(l=boxLength, w=boxWidth, h=lidHeight, r=cornerR, t=5);
    }

    // mount for knob/button
    translate([(boxLength-baseDepth/2)+(cornerR-wallThickness)+FUDGE,
        knobY,boxHeight/1.5])
      rotate([90,0,90])
        knobB(knobD=knobD, x=knobBase, cutX=baseCutX,
          cutY=baseCutY, z=baseDepth);
    // mount for the mic 
    translate([(boxLength-baseDepth/2)+(cornerR-wallThickness),
        micY,boxHeight-(micD/2+micFromTop+1)])
      rotate([90,0,90])
        micMount(D=micD, x=micBoardW, z=micMountT, md=micMountD);
    // Channels to hold the Nano in place
    translate([nanoBaseX/2, 0, wallThickness])
      rotate([0,0,180])
        NanoChannel(bx=nanoBaseX, baseD=nanoLift, notchD=nanoHoldA, holdD=0);
    translate([nanoBaseX/2, boardLength+tol-1.0, wallThickness])
      rotate([0,0,0])
        NanoChannel(bx=nanoBaseX, baseD=nanoLift, notchD=nanoHoldB);
    // Clips to battery holder
    translate([26.3+batXoff+clipWidth,batWidth+batYoff+clipWidth/2,wallThickness])
      rotate([0,0,180])
        Clip(width=clipWidth, clipStart=6.5, height=9.5);
    translate([26.3+batXoff+24.5,batWidth+batYoff+clipWidth/2,wallThickness])
      rotate([0,0,180])
        Clip(width=clipWidth, clipStart=6.5, height=9.5);
    // Loops to tie to dress
    translate([5,boxWidth+wallThickness+5,0])
      Loop();
    translate([57,boxWidth+wallThickness+5,0])
      Loop();
    translate([110,boxWidth+wallThickness+5,0])
      Loop();
  }
}

if(printCover) {
    Lid(l=boxLength, w=boxWidth, h=lidHeight, r=cornerR, t=5);
}
