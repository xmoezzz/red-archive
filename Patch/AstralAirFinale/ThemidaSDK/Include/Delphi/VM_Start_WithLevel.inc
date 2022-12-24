// VM_START_WITHLEVEL macro

// You can set virtualization level just changing the 12th byte (where the "XX" appears):

// asm DB $EB, $10, $57, $4c, $20, $20, $0C, $00, $00, $00, $00, XX, $00, $00, $57, $4c, $20, $20 end;

// The following example sets a virtualization level of 15

asm DB $EB, $10, $57, $4c, $20, $20, $0C, $00, $00, $00, $00, 15, $00, $00, $57, $4c, $20, $20 end;

