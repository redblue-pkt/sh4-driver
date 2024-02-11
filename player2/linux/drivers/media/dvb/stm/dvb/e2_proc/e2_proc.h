#ifndef E2_PROC_H_
#define E2_PROC_H_

int proc_video_aspect_get(void);
int proc_video_policy_get(void);
#if defined(QBOXHD) || defined(QBOXHD_MINI)
int proc_video_policy2_get(void);
#endif
int e2_proc_audio_getPassthrough(void);
int proc_avs_0_volume_write(struct file *file, const char __user *buf, unsigned long count, void *data);

#endif
