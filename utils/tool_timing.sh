#!/bin/bash
cat _x/link/vivado/vivado.log  | grep TNS
cat _x/link/vivado/vivado.log  | grep "The frequency is being automatically changed to"
cat _x/link/vivado/vivado.log  | grep scaled
