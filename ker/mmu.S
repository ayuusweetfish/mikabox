# r0 is the translation table base
.global mmu_enable
mmu_enable:
  mov   r1, #0
  // Invalidate d/i/unified caches
  mcr   p15, 0, r1, c7, c7, 0
  // Invalidate d/i/unified TLBs
  mcr   p15, 0, r1, c8, c7, 0
  // Table address
  orr   r0, #0x1b         // Mark page walk as inner and outer cacheable
  mcr   p15, 0, r0, c2, c0, 0
  mcr   p15, 0, r0, c2, c0, 1
  // Set domain control access to Manager
  mov   r1, #0xf
  mcr   p15, 0, r1, c3, c0, 0
  // Turn on MMU, with i/d caches
  mrc   p15, 0, r1, c1, c0, 0
  orr   r1, r1, #0xd      // M, C, W: MMU & d-cache
  orr   r1, r1, #0x1800   // Z, I: i-cache & branch prediction
  orr   r1, r1, #0x800000 // XP: disable subpage AP bits
  mcr   p15, 0, r1, c1, c0, 0
  bx    lr

.global mmu_flush
mmu_flush:
  mov   r0, #0
  mcr   p15, 0, r0, c8, c7, 0
  mcr   p15, 0, r0, c7, c10, 0  // Clean data cache
  bx    lr

.global mmu_domain_access_control
mmu_domain_access_control:
  mcr   p15, 0, r0, c3, c0, 0
  bx    lr

.global mmu_domain_access_control_get
mmu_domain_access_control_get:
  mrc   p15, 0, r0, c3, c0, 0
  bx    lr
