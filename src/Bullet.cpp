/*
 *    Source from Retrobattle, a NES-like collect-em-up.
 *    Copyright (C) 2010 Andreas Remar
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Original authors contact info: andreas.remar@gmail.com
 */

#include "Bullet.h"

Bullet::Bullet(remar2d *gfx, SoundManager *sfx)
  : Object(gfx, "shot", sfx), direction(LEFT)
{
  setAnimation("normal");
  setBoundingBox(6, 6, 0, 0);
  id = "Bullet";
}

void
Bullet::moveLeft()
{
  direction = LEFT;
}

void
Bullet::moveRight()
{
  direction = RIGHT;
}

void
Bullet::update()
{
  if(direction == LEFT)
    moveRel(-3, 0);
  else
    moveRel(3, 0);
}

Bullet::Direction
Bullet::getDirection()
{
  return direction;
}
