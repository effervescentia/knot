use super::ImportGraph;
use std::{
    collections::HashSet,
    fmt::{Display, Formatter},
};

impl Display for ImportGraph {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let mut roots = self.roots().collect::<Vec<_>>();
        let mut visited = HashSet::new();

        if roots.is_empty() {
            roots = self.graph.node_weights().copied().collect();
        }

        let printed = roots
            .into_iter()
            .map(|root| {
                if visited.contains(&root) {
                    Rows::empty()
                } else {
                    SubTree {
                        graph: self,
                        ancestors: vec![],
                        root,
                    }
                    .format(&mut visited)
                }
            })
            .collect::<Vec<_>>();
        let max_depth = printed
            .iter()
            .map(|x| x.rows.len())
            .max()
            .unwrap_or_default();

        f.write_str(
            Rows::merge(max_depth, &printed)
                .iter()
                .map(|x| x.trim_end())
                .collect::<Vec<_>>()
                .join("\n")
                .as_str(),
        )
    }
}

fn pad(value: &str, width: usize) -> String {
    format!("{:<1$}", value, width)
}

struct SubTree<'a> {
    graph: &'a ImportGraph,
    ancestors: Vec<usize>,
    root: usize,
}

impl<'a> SubTree<'a> {
    fn format_rows(rows: Vec<String>) -> Rows {
        let max_width = rows
            .iter()
            .map(|x| x.len())
            .max()
            .map(|x| 2.max(x + 1))
            .unwrap();

        Rows {
            width: max_width,
            rows: rows.iter().map(|x| pad(x, max_width)).collect(),
        }
    }

    fn format_dividers(subtrees: Vec<Rows>) -> Vec<String> {
        vec![subtrees
            .iter()
            .enumerate()
            .map(|(index, x)| {
                if index < subtrees.len() - 1 {
                    vec![
                        String::from(if index == 0 { "|" } else { "," }),
                        String::from("-").repeat(x.width - 1),
                    ]
                    .concat()
                } else {
                    String::from(",")
                }
            })
            .collect::<Vec<_>>()
            .concat()]
    }

    fn format_children(&self, children: Vec<usize>, visited: &'a mut HashSet<usize>) -> Vec<Rows> {
        children
            .into_iter()
            .map(|child| {
                if self.ancestors.contains(&&child) {
                    Self::format_rows(vec![format!("cycle({child})")])
                } else {
                    let mut ancestors = self.ancestors.clone();
                    ancestors.push(self.root);

                    Self {
                        graph: self.graph,
                        ancestors,
                        root: child,
                    }
                    .format(visited)
                }
            })
            .collect()
    }

    fn format(self, visited: &'a mut HashSet<usize>) -> Rows {
        let children = self.graph.children(&self.root).collect::<Vec<_>>();

        let subtrees = match (visited.contains(&self.root), children.is_empty()) {
            (true, true) => vec![],

            (true, false) => vec![Rows::from("[…]")],

            (false, _) => {
                visited.insert(self.root);

                self.format_children(children, visited)
            }
        };

        let has_multiple_subtrees = subtrees.len() > 1;
        let max_depth = subtrees
            .iter()
            .map(|x| x.rows.len())
            .max()
            .unwrap_or_default();

        let root_row = self.root.to_string();
        let pipe_row = subtrees
            .iter()
            .map(|x| pad("|", x.width))
            .collect::<Vec<_>>()
            .concat();
        let child_rows = Rows::merge(max_depth, &subtrees);
        let divider_rows = if has_multiple_subtrees {
            Self::format_dividers(subtrees)
        } else {
            vec![]
        };

        Self::format_rows(vec![vec![root_row], divider_rows, vec![pipe_row], child_rows].concat())
    }
}

struct Rows {
    width: usize,
    rows: Vec<String>,
}

impl Rows {
    fn empty() -> Self {
        Self {
            width: 0,
            rows: vec![],
        }
    }

    fn merge(depth: usize, xs: &Vec<Self>) -> Vec<String> {
        (0..depth)
            .into_iter()
            .map(|index| {
                xs.iter()
                    .map(|x| {
                        let value = if x.rows.len() > index {
                            x.rows.get(index).unwrap()
                        } else {
                            ""
                        };

                        pad(value, x.width)
                    })
                    .collect::<Vec<_>>()
                    .concat()
            })
            .collect()
    }
}

impl From<&str> for Rows {
    fn from(value: &str) -> Self {
        Self {
            width: value.len(),
            rows: vec![value.to_string()],
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::link::ImportGraph;
    use kore::assert_str_eq;

    #[test]
    fn roots() {
        let graph = ImportGraph::from_nodes(vec![0, 1, 2]);

        assert_str_eq!(
            graph.to_string(),
            "0 1 2
"
        );
    }

    #[test]
    fn deep() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 3)]);

        assert_str_eq!(
            graph.to_string(),
            "0
|
1
|
2
|
3
"
        );
    }

    #[test]
    fn wide() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (0, 2), (0, 3)]);

        assert_str_eq!(
            graph.to_string(),
            "0
|-,-,
| | |
3 2 1
"
        );
    }

    #[test]
    fn cyclic() {
        let graph = ImportGraph::from_edges(vec![
            (0, 1),
            (1, 2),
            (1, 6),
            (2, 0),
            (3, 4),
            (4, 3),
            (5, 6),
            (6, 1),
        ]);

        assert_str_eq!(
            graph.to_string(),
            "5
|
6
|
1
|--------,
|        |
cycle(6) 2
         |
         0
         |
         cycle(1)"
        );
    }
}
