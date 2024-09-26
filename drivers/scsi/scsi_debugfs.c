// SPDX-License-Identifier: GPL-2.0
#include <linux/bitops.h>
#include <linux/cleanup.h>
#include <linux/seq_file.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_host.h>
#include "scsi_debugfs.h"

#define SCSI_CMD_FLAG_NAME(name)[const_ilog2(SCMD_##name)] = #name
static const char *const scsi_cmd_flags[] = {
	SCSI_CMD_FLAG_NAME(TAGGED),
	SCSI_CMD_FLAG_NAME(INITIALIZED),
	SCSI_CMD_FLAG_NAME(LAST),
};
#undef SCSI_CMD_FLAG_NAME

static int scsi_flags_show(struct seq_file *m, const unsigned long flags,
			   const char *const *flag_name, int flag_name_count)
{
	bool sep = false;
	int i;

	for_each_set_bit(i, &flags, BITS_PER_LONG) {
		if (sep)
			seq_puts(m, "|");
		sep = true;
		if (i < flag_name_count && flag_name[i])
			seq_puts(m, flag_name[i]);
		else
			seq_printf(m, "%d", i);
	}
	return 0;
}

static const char *scsi_cmd_list_info(struct scsi_cmnd *cmd)
{
	struct Scsi_Host *shost = cmd->device->host;
	struct scsi_cmnd *cmd2;

	guard(spinlock_irq)(shost->host_lock);

	list_for_each_entry(cmd2, &shost->eh_abort_list, eh_entry)
		if (cmd == cmd2)
			return "on eh_abort_list";

	list_for_each_entry(cmd2, &shost->eh_cmd_q, eh_entry)
		if (cmd == cmd2)
			return "on eh_cmd_q";

	return NULL;
}

void scsi_show_rq(struct seq_file *m, struct request *rq)
{
	struct scsi_cmnd *cmd = blk_mq_rq_to_pdu(rq);
	int alloc_ms = jiffies_to_msecs(jiffies - cmd->jiffies_at_alloc);
	int timeout_ms = jiffies_to_msecs(rq->timeout);
	char buf[80] = "(?)";

	if (cmd->flags & SCMD_INITIALIZED) {
		const char *list_info = scsi_cmd_list_info(cmd);

		__scsi_format_command(buf, sizeof(buf), cmd->cmnd, cmd->cmd_len);
		seq_printf(m, ", .cmd=%s, .retries=%d, .allowed=%d, .result = %#x%s%s",
			   buf, cmd->retries, cmd->allowed, cmd->result,
			   list_info ? ", " : "", list_info ? : "");
		seq_printf(m, ", .timeout=%d.%03d, allocated %d.%03d s ago",
			   timeout_ms / 1000, timeout_ms % 1000,
			   alloc_ms / 1000, alloc_ms % 1000);
	}
	seq_printf(m, ", .flags=");
	scsi_flags_show(m, cmd->flags, scsi_cmd_flags,
			ARRAY_SIZE(scsi_cmd_flags));
}
