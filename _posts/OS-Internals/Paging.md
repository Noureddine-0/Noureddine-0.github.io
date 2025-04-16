---
title: "How Your CPU Lies to You: The Brutal Truth About Linear-to-Physical Translation"
classes: wide
header:
  teaser: /assets/OS-Internals/paging/CPU.jpG
ribbon: DodgerBlue
description: "Ever wonder how your computer can run multiple large applications simultaneously, even if their combined memory requirement exceeds your physical RAM? Or how your operating system keeps one buggy program from crashing the entire system"
categories:
  - OS Internals
toc: true
---

Ever wonder how your computer can run multiple large applications simultaneously, even if their combined memory requirement exceeds your physical RAM? Or how your operating system keeps one buggy program from crashing the entire system by messing with another program's memory? The answer, in large part, lies in a fundamental operating system concept: **Paging**.

Paging is a cornerstone of modern memory management. It's the clever trick that allows operating systems to provide processes with their own private, large address spaces, efficiently manage physical memory, and enable features like virtual memory.

Let's dive deep into the world of paging.

# The Problem: Managing Limited Physical Memory

Before paging, memory allocation was plagued by **fragmentation** and **rigidity**. Programs demanded contiguous blocks of physical RAM, creating scattered gaps of unusable memory as processes loaded and exited. This forced painful **memory shuffling** to consolidate space. Worse, with no isolation, a single program could crash or corrupt the entire system. Memory expansion was nearly impossible, and swapping entire programs to disk was brutally inefficient. The system either wasted space or wasted time—often both.

The lack of **memory protection** meant every process had direct access to physical RAM, turning bugs into system-wide disasters. Multitasking was a fragile balancing act, where one misbehaving program could overwrite another’s data—or even the OS itself. Without virtual memory, programs were limited to the actual RAM available, forcing developers to write agonizingly optimized code. Memory management wasn’t just hard—it was **dangerous**.

Paging was developed to solve these critical issues.

# What is Paging? The Core Idea

Paging is a memory management technique that **decouples** a program’s view of memory (virtual addresses) from the actual physical RAM. Instead of requiring programs to fit into contiguous blocks of physical memory, the system divides memory into fixed-size chunks called **pages** (typically 4KB). Each program operates in its own virtual address space, blissfully unaware of where—or even if—its pages reside in physical RAM. The operating system, CPU and mainly the MMU collaborate to translate these virtual addresses into physical locations on the fly, using a hidden lookup system called **page tables**.

The real magic lies in **abstraction and flexibility**. Since programs no longer depend on physical memory layouts, the OS can dynamically map pages to available RAM or even to disk (swap space) when physical memory runs low. This enables **isolation** (one program can’t crash another), **efficient memory** use (no more fragmentation headaches), and the illusion of **near-infinite memory—all** while keeping the hardware complexities invisible to applications. Paging doesn’t just solve old problems; it fundamentally changes how memory works, making modern multitasking, security, and large-scale software possible.

At its heart, paging is a **translation layer**—a way for the CPU and OS to lie convincingly to programs, letting them run as if they own the entire machine while quietly managing the chaos behind the scenes. Whether it’s loading code on demand, protecting kernel memory, or enabling shared libraries, paging is the silent enabler of almost everything in modern computing.

Now , lets see hoe paging works.

# How Does Paging Work? Address Translation

## Critical Registers for Paging: The Hardware Control Panel

To truly grasp paging, we need to understand the key registers that orchestrate address translation.Now u need just to know that they esist , it is after we explain how address translation works u should understand each one of them. These registers configure the Memory Management Unit (MMU), define page table hierarchies, and enforce memory access rules. Here are the most important ones:

**1. CR3 (Control Register 3) - The Root of All Paging**

  - CR3 (Control Register 3) is the CPU's gateway to paging. It stores the physical address of the **top-level page table** (like PML4 in x86-64), which the MMU uses to translate virtual addresses. Every process has its own CR3 value, that's why even we see two process acess what is seems the same linear address it is in reality two different one.

**2. CR0 (Control Register 0) Paging Enable switch**

   - Bit 31 (PG): Enables/disabled paging. If cleared, **linear addresses = physical addresses**.
   - Bit 16 (WP): When set, inhibits supervisor-level procedures from writing into readonly pages; when clear, allows supervisor-level procedures to write into read-only pages.

**3. CR4 (Control Register 4) Paging features**

   - Bit 4 (PSE): PSE (Page Size Extension) enables 4MB **"huge pages"** (vs standard 4KB), reducing TLB cache pressure by needing just 1 entry per large allocation instead of hundreds. This dramatically cuts page walk overhead for big memory blocks while maintaining backward compatibility with 4KB pages.
   - Bit 5 (PAE): PAE (Physical Address Extension) Allows a 32-bit operating system to access more than 4 gigabyte of memory by extending the physical address space from 32 bits to 36 bits.
   - Bit 7 (PGE): PGE (Page Global Enable) , enables support for global pages, which are mostly used for kernel code and data so they remain cached in the TLB across context switches, improving performance.
   - Bit 20 (SMEP): SMEP (Supervisor Mode Execution Prevention) blocks  kernel-mode code from executing instructions located in user-space pages. This enhances security by preventing execution of injected user-mode payloads in kernel context.
   - Bit 21 (SMAP): SMAP (Supervisor Mode Access Prevention) prevents supervisor-mode (kernel) from accessing user-space memory.
**4. IA32_EFER (Extended Feature Enable Register) Model-Specific-Register**
   In this MSR register , we're only interested in 3 bits , but since the used bits are only 4 and the others are reserved, let's explain each one of them :
   - Bit 0 (SCE): SCE (SysCall Enable) enables the SYSCALL/SYSRET instructions for faster user-to-kernel transitions in 32-bit and 64-bit modes.
   - Bit 8 (LME): LME (Long Mode Enable) enables Long Mode, which is required for 64-bit operation on Intel and AMD CPUs. Setting this bit prepares the CPU to enter 64-bit mode, but it doesn't take effect until CR0.PG (Paging Enable) is also set.
   - Bit 9 (LMA): LMA (Long Mode Access) indicates whether the CPU is currently operating in 64-bit long mode.It is Readable only.
   - Bit 11 (NXE): NXE (No Execute Enable) activates hardware-enforced memory protection by honoring the NX bit in page tables to block code execution in data pages.

   [![1](/assets/OS-Internals/paging/IA32_EFER.PNG)](/assets/images/malware-analysis/gcleaner/IA32_EFER.PNG)
## Paging Modes
When **CR0.PG = 0**, paging is **disabled**, and the CPU treats all linear addresses as physical addresses. In this state, features like CR4.PAE, IA32_EFER.LME, CR0.WP, CR4.PSE, CR4.PGE, CR4.SMEP, CR4.SMAP, and IA32_EFER.NXE are ignored.

To enable paging **CR0 must be set to 1** , but **WP must be set too**.Once active, the paging mode depends on CR4.PAE and IA32_EFERE.LME:

   - If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used, and we have a Max of 4GB address space.
   - If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, PAE paging is used and we have a max of 64GB address space.
	- If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1, IA-32e paging is used and theorietically we have 16EB.However, real-world limits are much lower due to hardware and architectural constraints.


## Paging-Mode Enabling
- IA32_EFER.LME cannot be modified while paging is enabled (CR0.PG = 1). Attempts to do so using WRMSR cause a general-protection exception (#GP(0)).
- Paging cannot be enabled (by setting CR0.PG to 1) while CR4.PAE = 0 and IA32_EFER.LME = 1. Attempts to do so using MOV to CR0 cause a general-protection exception (#GP(0)). 
- Regardless of the current paging mode, software can disable paging by clearing CR0.PG with MOV to CR0.
- Software can make transitions between 32-bit paging and PAE paging by changing the value of CR4.PAE with MOV to CR4.
- Software cannot make transitions directly between IA-32e paging and either of the other two paging modes. It must first disable paging (by clearing CR0.PG with MOV to CR0), then set CR4.PAE and IA32_EFER.LME to the desired values (with MOV to CR4 and WRMSR), and then re-enable paging (by setting CR0.PG with MOV to CR0).
The following image sum-up all this:


[![2](/assets/OS-Internals/paging/paging-mode.PNG)](/assets/images/malware-analysis/gcleaner/paging-mode.PNG)


### HIERARCHICAL PAGING STRUCTURES

All three paging modes use multi-level page tables to convert linear addresses to physical addresses. Page tables are always 4KB in size, containing either 1024 32-bit entries (32-bit mode) or 512 64-bit entries (PAE/IA-32e modes). The CPU splits linear addresses into two parts: upper bits index successive page table entries (starting from CR3), while lower bits become the final page offset. Each entry either points to another table or directly to a  physical page frame. Translation works like a multi-stage lookup: the CPU walks the hierarchy until finding a page frame, combining its address with the offset to get the final physical address.

**Page translation begins at the physical address stored in CR3**. The CPU performs an iterative lookup: the highest bits of the linear address index an entry in the current page table (starting with CR3). If the entry points to another table, the CPU repeats the process with the next bits. If it points to a page frame, the lookup ends—the entry’s address becomes the base of the 4KB frame, and the remaining bits serve as the offset within it.

If more than 12 bits remain in the linear address, bit 7 (PS — page size) of the current paging-structure entry is consulted. If the bit is 0, the entry references another paging structure; if the bit is 1, the entry maps a page.If only 12 bits remain in the linear address, the current paging-structure entry always maps a page (bit 7 is used for other purposes).

### 32-Bit Paging

[![3](/assets/OS-Internals/paging/32pagingwithoutps.PNG)](/assets/images/malware-analysis/gcleaner/32pagingwithoutps.PNG)

The Image above shows how address translation works when the page directory entry PS bit is cleared.If CR4.PSE =1 and the the PDE (page directory entry) , the next 22 bits maps a whole page frame as shown below.

[![3](/assets/OS-Internals/paging/32pagingwithps.PNG)](/assets/images/malware-analysis/gcleaner/32pagingwithps.PNG)

