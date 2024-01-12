typedef unsigned long (*tb_get_ticks_fn)(void);

struct tb_ctx {
	unsigned long last_tick_ms;
	unsigned long target_ns, target_ms;
	unsigned long acc_ticks_ns;
	tb_get_ticks_fn get_ticks_ms;
};

void tb_init(struct tb_ctx *c, unsigned long target_ns,
		tb_get_ticks_fn get_ticks_ms);
void tb_reset(struct tb_ctx *c);
unsigned tb_frame_trigger(struct tb_ctx *c, unsigned *rem_ms);
