## CPUID instruction
A way to check availability of features in our CPU.

Instead of operands it uses EAX and sometimes ECX as input.

Output goes to ax, bx, cx, dx, and the CPU support this operation if ID flag (bit 21) is on.


-----

## Execution modes

#### Real-address mode
The oldest and simplest. No virtual memory and no rings, runs in 16 bit.

#### Protected mode
Main mode that OS run in today. 

#### IA-32e mode
Also referred as long/extended mode.

#### System management mode
If SMI (system management interrupt) received, we are getting to this mode.

Can protect itself from all other privileged code.

Currently does not seem relevant/interesting 

##### Diagram that shows relevant parts in execution modes

![alt text](image.png)

----

## Privilige Rings

![alt text](image-1.png)

* Note that most OSs only have 2 rings (instead of 4 showen in the picture). Ring 0-2 serves as ring 0 (kernel mode) and ring 3 as ring 1 (user mode)

## Segmentation
A concept that is being used in order to divide the process memory space into a few logical segments. 

These are the commonly used segments:

Code segment  - includes the program instructions

Data segment  - global and static variables 

Stack segment - function call stack

Extra segment - extra (usually data)


Each segment have a segment descriptor, that would include base address, size and access.

With segments, logical addresses came by. They are shaped in the form of `SEGMENT+offset`.

##### Logical address with 64-bit Operand Size
<table>
  <tr>
    <th>16-bit Segment Selector<br><sub>Bits 79–64</sub></th>
    <th>64-bit Offset<br><sub>Bits 63–0</sub></th>
  </tr>
</table>

##### Logical address with 32-bit Operand Size
<table>
  <tr>
    <th>16-bit Segment Selector<br><sub>Bits 47–32</sub></th>
    <th>32-bit Offset<br><sub>Bits 31–0</sub></th>
  </tr>
</table>

##### Linear address with 32-bit Operand Size
<table>
  <tr>
    <th>32-bit Offset<br><sub>Bits 31–0</sub></th>
  </tr>
</table>

More in-depth, in order to transform logical address to linear, the `segment selecotor` needs to find in a descriptor table its base address and from there to add the `offset`.

That descriptor table is also known as the **GDT** (Global Descriptor Table)

If we will take a deeper dive into the segment selector, 
we will se it is built in the following form:
<table>
  <tr>
    <th>Index<br><sub>Bits 3-15</sub></th>
    <th>GDT/LDT<br><sub>Bit 2</sub></th>
    <th>RPL<br><sub>Bits 0-1</sub></th>
  </tr>
</table>

 Index - the row umber in the table

 GDT(=0)/LDT(=1) -  from which table it should look for, local or global

 RPL - Requested Privilege Level

## GDT
