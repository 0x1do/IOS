# Ido Operating System

An educational operating system implementation project for the **Bagrut in Cyber** program.

---

## Project Overview

Following my teacher’s advice, I decided to split the project into multiple components.  
For each component, I’ll design **two versions**:

1. **Minimal Solution** – a simple, working implementation.  
2. **Enhanced Solution** – a more advanced version with extra features or optimizations.

A detailed breakdown table will be added soon.

---

## Current Progress

I’ve successfully built a **64 bit minimal OS** that **Limine can boot** and a simple implementation of printk (that gets framebuffer (not VGA!!!) and supports format string), segments, simple interrupts and physical page allocator using simple bitmap and each chunk have a metadata containing its size field

---

## Next Steps


paging
filesystem
schedular
small usermode - add another ring, syscalls, etc
drivers - keyboard/mouse/screen

Maybe rewrite in C++
