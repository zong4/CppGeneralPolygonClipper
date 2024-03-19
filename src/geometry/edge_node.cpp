#include "edge_node.hpp"

// void gpc::insert_bound(gpc::edge_node **b, gpc::edge_node *e) {
//   edge_node *existing_bound;

//   if (!*b) {
//     /* Link node e to the tail of the list */
//     *b = e;
//   } else {
//     /* Do primary sort on the x field */
//     if (e->bot.x < (*b)->bot.x) {
//       /* Insert a new node mid-list */
//       existing_bound = *b;
//       *b = e;
//       (*b)->next_bound = existing_bound;
//     } else {
//       if (e->bot.x == (*b)->bot.x) {
//         /* Do secondary sort on the dx field */
//         if (e->dx < (*b)->dx) {
//           /* Insert a new node mid-list */
//           existing_bound = *b;
//           *b = e;
//           (*b)->next_bound = existing_bound;
//         } else {
//           /* Head further down the list */
//           insert_bound(&((*b)->next_bound), e);
//         }
//       } else {
//         /* Head further down the list */
//         insert_bound(&((*b)->next_bound), e);
//       }
//     }
//   }
// }