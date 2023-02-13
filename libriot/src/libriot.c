#include "libriot.h"

extern inline void
riot_intrusive_list_node_link(struct riot_intrusive_list_node *self,
			      struct riot_intrusive_list_node *prev,
			      struct riot_intrusive_list_node *next);

extern inline void
riot_intrusive_list_node_snip(struct riot_intrusive_list_node *self);

extern inline void
riot_intrusive_list_init(struct riot_intrusive_list *self);

extern inline void
riot_intrusive_list_push(struct riot_intrusive_list *self,
			 struct riot_intrusive_list_node *elem);
