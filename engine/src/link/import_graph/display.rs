use super::ImportGraph;
use kore::{invariant, str};
use std::{
    collections::HashSet,
    fmt::{Display, Formatter, Write},
};

const GAP: usize = 1;

impl Display for ImportGraph {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        Graph::from_import_graph(self).fmt(f)
    }
}

struct Node {
    width: usize,
    value: String,
}

impl Node {
    pub fn from_string(value: String) -> Self {
        Self {
            width: value.len(),
            value,
        }
    }
}

impl Display for Node {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        f.write_str(&self.value)
    }
}

struct Tree {
    root: Node,
    children: Vec<Tree>,
}

impl Tree {
    fn from_root(root: String) -> Self {
        let node = Node::from_string(root);

        Self {
            root: node,
            children: vec![],
        }
    }

    fn from_graph(root: usize, graph: &ImportGraph, visited: &mut HashSet<usize>) -> Self {
        if visited.contains(&root) {
            return Self::from_root(format!("cycle({root})"));
        }

        visited.insert(root);

        let children: Vec<Self> = graph
            .children(&root)
            .map(|x| Self::from_graph(x, graph, visited))
            .collect();

        let node = Node::from_string(root.to_string());
        Self {
            root: node,
            children,
        }
    }

    fn root_row(&self) -> Row {
        Row {
            width: self.root.width,
            segments: vec![self.root.value.clone()],
        }
    }

    fn dividers(children: &[Block]) -> Block {
        if children.len() == 1 {
            return Block::from_rows(vec![Row {
                width: 1,
                segments: vec![str!("|")],
            }]);
        }

        let (horizontal_pipe, vertical_pipes) =
            children
                .iter()
                .enumerate()
                .fold((vec![], vec![]), |mut acc, (index, _)| {
                    if index == 0 {
                        acc.0.push(str!("|"));
                        acc.1.push(str!("|"));
                    } else {
                        let prev_width =
                            children.get(index - 1).map(|x| x.width).unwrap_or_default();

                        acc.0.push(format!("{:->1$}", ",", prev_width + GAP));
                        acc.1.push(format!("{:>1$}", "|", prev_width + GAP));
                    }

                    acc
                });

        Block::from_rows(vec![
            Row {
                width: horizontal_pipe.join("").len(),
                segments: horizontal_pipe,
            },
            Row {
                width: vertical_pipes.join("").len(),
                segments: vertical_pipes,
            },
        ])
    }

    fn to_block(&self) -> Block {
        let block = Block::from_rows(vec![self.root_row()]);

        if self.children.is_empty() {
            return block;
        }

        let child_blocks = self.children.iter().map(Self::to_block).collect::<Vec<_>>();

        block
            .append_vertical(&Self::dividers(&child_blocks))
            .append_vertical(&Block::layout(child_blocks, GAP))
    }
}

struct Graph {
    trees: Vec<Tree>,
}

impl Graph {
    fn from_import_graph(import_graph: &ImportGraph) -> Self {
        // nodes that don't have any parents
        let mut roots = import_graph.roots().collect::<Vec<_>>();
        let mut visited = HashSet::new();

        // the entire graph is empty or a cycle
        if roots.is_empty() {
            roots = import_graph.graph.node_weights().copied().collect();
        }

        let trees = roots
            .iter()
            .map(|x| Tree::from_graph(*x, import_graph, &mut visited))
            .collect::<Vec<_>>();

        Self { trees }
    }

    fn to_block(&self) -> Block {
        Block::layout(self.trees.iter().map(Tree::to_block), GAP)
    }
}

impl Display for Graph {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.to_block().fmt(f)
    }
}

#[derive(Clone, Debug, PartialEq)]
struct Row {
    width: usize,
    segments: Vec<String>,
}

impl Row {
    fn padding(width: usize) -> Self {
        Self {
            width,
            segments: vec![str!(" ").repeat(width)],
        }
    }

    fn pad_left(&self, offset: usize) -> Self {
        if offset == 0 {
            self.clone()
        } else {
            Self::padding(offset).merge(self)
        }
    }

    fn pad_right(&self, offset: usize) -> Self {
        if offset == 0 {
            self.clone()
        } else {
            self.merge(&Self::padding(offset))
        }
    }

    fn merge(&self, other: &Self) -> Self {
        Self {
            width: self.width + other.width,
            segments: [self.segments.clone(), other.segments.clone()].concat(),
        }
    }
}

impl Display for Row {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        for x in &self.segments {
            f.write_str(x)?;
        }

        f.write_char('\n')
    }
}

#[derive(Clone, Debug, PartialEq)]
struct Block {
    width: usize,
    rows: Vec<Row>,
}

impl Block {
    const fn empty() -> Self {
        Self {
            width: 0,
            rows: vec![],
        }
    }

    fn padding(width: usize) -> Self {
        Self {
            width,
            rows: vec![Row::padding(width)],
        }
    }

    fn from_rows(rows: Vec<Row>) -> Self {
        let width = rows.iter().map(|x| x.width).max().unwrap_or_default();

        Self { width, rows }
    }

    const fn is_empty(&self) -> bool {
        self.width == 0
    }

    fn normalize(self) -> Self {
        let max_width = self.rows.iter().map(|x| x.width).max().unwrap_or_default();

        Self {
            width: max_width,
            rows: self
                .rows
                .into_iter()
                .map(|x| {
                    if x.width < max_width {
                        x.pad_right(max_width - x.width)
                    } else {
                        x
                    }
                })
                .collect(),
        }
    }

    fn append_vertical(&self, other: &Self) -> Self {
        Self::from_rows([self.rows.clone(), other.rows.clone()].concat())
    }

    fn append_horizontal(self, rhs: &Self) -> Self {
        let lhs = self.normalize();
        let width = lhs.width + rhs.width;
        let height = lhs.rows.len().max(rhs.rows.len());
        let mut rows = vec![];

        for i in 0..height {
            match (lhs.rows.get(i), rhs.rows.get(i)) {
                (Some(l), Some(r)) => rows.push(l.merge(r)),

                (Some(x), None) => rows.push(x.clone()),

                (None, Some(x)) => rows.push(x.pad_left(lhs.width)),

                (None, None) => invariant!("should never iterate past both rows"),
            }
        }

        Self { width, rows }
    }

    fn layout<T: IntoIterator<Item = Self>>(xs: T, gap: usize) -> Self {
        xs.into_iter().fold(Self::empty(), |acc, x| {
            if acc.is_empty() {
                acc.append_horizontal(&x)
            } else {
                acc.append_horizontal(&Self::padding(gap))
                    .append_horizontal(&x)
            }
        })
    }
}

impl Display for Block {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.rows
            .iter()
            .try_fold((), |_, x| f.write_str(&x.to_string()))
    }
}

#[cfg(test)]
mod tests {
    use super::ImportGraph;
    use kore::{assert_str_eq, str};

    #[test]
    fn empty() {
        let graph = ImportGraph::from_nodes(&[]);

        assert_str_eq!(graph.to_string(), "");
    }

    #[test]
    fn roots() {
        let graph = ImportGraph::from_nodes(&[0, 1, 2]);

        assert_str_eq!(
            graph.to_string(),
            "0 1 2
"
        );
    }

    #[test]
    fn deep() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 3)]);

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
    fn branching() {
        let graph = ImportGraph::from_edges(&[(0, 1), (0, 2)]);

        assert_str_eq!(
            graph.to_string(),
            "0
|-,
| |
2 1
"
        );
    }

    #[test]
    fn wide() {
        let graph = ImportGraph::from_edges(&[(0, 1), (0, 2), (0, 3), (1, 6)]);

        assert_str_eq!(
            graph.to_string(),
            "0
|-,-,
| | |
3 2 1
    |
    6
"
        );
    }

    #[test]
    fn cyclic() {
        let graph = ImportGraph::from_edges(&[
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
         cycle(1)
"
        );
    }

    mod tree {
        use super::super::{Block, Node, Row, Tree};
        use super::*;

        fn mock_block(value: String) -> Block {
            Block {
                width: value.len(),
                rows: vec![Row {
                    width: value.len(),
                    segments: vec![value],
                }],
            }
        }

        #[test]
        fn empty_dividers() {
            assert_eq!(
                Tree::dividers(&[]),
                Block {
                    width: 0,
                    rows: vec![
                        Row {
                            width: 0,
                            segments: vec![]
                        },
                        Row {
                            width: 0,
                            segments: vec![]
                        }
                    ]
                }
            );
        }

        #[test]
        fn thin_divider_row() {
            assert_eq!(
                Tree::dividers(&[mock_block(str!("x"))]),
                Block {
                    width: 1,
                    rows: vec![Row {
                        width: 1,
                        segments: vec![str!("|")]
                    }]
                }
            );
        }

        #[test]
        fn wide_divider_row() {
            assert_eq!(
                Tree::dividers(&[mock_block(str!("foo"))]),
                Block {
                    width: 1,
                    rows: vec![Row {
                        width: 1,
                        segments: vec![str!("|")]
                    }]
                }
            );
        }

        #[test]
        fn multiple_divider_row() {
            assert_eq!(
                Tree::dividers(&[
                    mock_block(str!("a")),
                    mock_block(str!("b")),
                    mock_block(str!("c"))
                ]),
                Block {
                    width: 5,
                    rows: vec![
                        Row {
                            width: 5,
                            segments: vec![str!("|"), str!("-,"), str!("-,")]
                        },
                        Row {
                            width: 5,
                            segments: vec![str!("|"), str!(" |"), str!(" |")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn uneven_divider_row() {
            assert_eq!(
                Tree::dividers(&[
                    mock_block(str!("fa")),
                    mock_block(str!("laa")),
                    mock_block(str!("la")),
                ]),
                Block {
                    width: 8,
                    rows: vec![
                        Row {
                            width: 8,
                            segments: vec![str!("|"), str!("--,"), str!("---,")]
                        },
                        Row {
                            width: 8,
                            segments: vec![str!("|"), str!("  |"), str!("   |")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn root_node_to_block() {
            let result = Tree {
                root: Node::from_string(str!("foo")),
                children: vec![],
            }
            .to_block();

            assert_eq!(
                result,
                Block {
                    width: 3,
                    rows: vec![Row {
                        width: 3,
                        segments: vec![str!("foo")]
                    }]
                }
            );
        }

        #[test]
        fn wide_tree_to_block() {
            let result = Tree {
                root: Node::from_string(str!("foo")),
                children: vec![
                    Tree {
                        root: Node::from_string(str!("bar")),
                        children: vec![],
                    },
                    Tree {
                        root: Node::from_string(str!("fizz")),
                        children: vec![],
                    },
                    Tree {
                        root: Node::from_string(str!("buzz")),
                        children: vec![],
                    },
                ],
            }
            .to_block();

            assert_eq!(
                result,
                Block {
                    width: 13,
                    rows: vec![
                        Row {
                            width: 3,
                            segments: vec![str!("foo")]
                        },
                        Row {
                            width: 10,
                            segments: vec![str!("|"), str!("---,"), str!("----,")]
                        },
                        Row {
                            width: 10,
                            segments: vec![str!("|"), str!("   |"), str!("    |")]
                        },
                        Row {
                            width: 13,
                            segments: vec![
                                str!("bar"),
                                str!(" "),
                                str!("fizz"),
                                str!(" "),
                                str!("buzz")
                            ]
                        }
                    ]
                }
            );
        }
    }

    mod row {
        use super::super::Row;
        use super::*;

        fn mock_row(value: String) -> Row {
            Row {
                width: value.len(),
                segments: vec![value],
            }
        }

        #[test]
        fn pad_left() {
            let row = mock_row(str!("foo"));

            assert_eq!(
                row.pad_left(3),
                Row {
                    width: 6,
                    segments: vec![str!("   "), str!("foo")]
                }
            );
        }

        #[test]
        fn pad_right() {
            let row = mock_row(str!("foo"));

            assert_eq!(
                row.pad_right(3),
                Row {
                    width: 6,
                    segments: vec![str!("foo"), str!("   ")]
                }
            );
        }

        #[test]
        fn merge() {
            let lhs = mock_row(str!("foo"));
            let rhs = mock_row(str!("bar"));

            assert_eq!(
                lhs.merge(&rhs),
                Row {
                    width: 6,
                    segments: vec![str!("foo"), str!("bar")]
                }
            );
        }
    }

    mod block {
        use super::super::{Block, Row};
        use super::*;

        fn mock_block(rows: Vec<Vec<String>>) -> Block {
            Block::from_rows(
                rows.into_iter()
                    .map(|x| Row {
                        width: x.iter().map(String::len).sum(),
                        segments: x,
                    })
                    .collect(),
            )
        }

        #[test]
        fn append_vertical() {
            let top = mock_block(vec![
                vec![str!("foo"), str!("bar")],
                vec![str!("fizz"), str!("buzz")],
            ]);
            let bottom = mock_block(vec![
                vec![str!("flip"), str!("flop")],
                vec![str!("zip"), str!("zap")],
            ]);

            assert_eq!(
                top.append_vertical(&bottom),
                Block {
                    width: 8,
                    rows: vec![
                        Row {
                            width: 6,
                            segments: vec![str!("foo"), str!("bar")]
                        },
                        Row {
                            width: 8,
                            segments: vec![str!("fizz"), str!("buzz")]
                        },
                        Row {
                            width: 8,
                            segments: vec![str!("flip"), str!("flop")]
                        },
                        Row {
                            width: 6,
                            segments: vec![str!("zip"), str!("zap")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn append_horizontal_simple() {
            let lhs = mock_block(vec![vec![str!("foo")]]);
            let rhs = mock_block(vec![vec![str!("bar")]]);

            assert_eq!(
                lhs.append_horizontal(&rhs),
                Block {
                    width: 6,
                    rows: vec![Row {
                        width: 6,
                        segments: vec![str!("foo"), str!("bar")]
                    }]
                }
            );
        }

        #[test]
        fn append_horizontal_multiline() {
            let lhs = mock_block(vec![vec![str!("foo")], vec![str!("bar")]]);
            let rhs = mock_block(vec![vec![str!("fizz")], vec![str!("buzz")]]);

            assert_eq!(
                lhs.append_horizontal(&rhs),
                Block {
                    width: 7,
                    rows: vec![
                        Row {
                            width: 7,
                            segments: vec![str!("foo"), str!("fizz")]
                        },
                        Row {
                            width: 7,
                            segments: vec![str!("bar"), str!("buzz")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn append_horizontal_raw_edge() {
            let lhs = mock_block(vec![vec![str!("a")], vec![str!("foo")], vec![str!("fizz")]]);
            let rhs = mock_block(vec![vec![str!("b")], vec![str!("bar")], vec![str!("buzz")]]);

            assert_eq!(
                lhs.append_horizontal(&rhs),
                Block {
                    width: 8,
                    rows: vec![
                        Row {
                            width: 5,
                            segments: vec![str!("a"), str!("   "), str!("b")]
                        },
                        Row {
                            width: 7,
                            segments: vec![str!("foo"), str!(" "), str!("bar")]
                        },
                        Row {
                            width: 8,
                            segments: vec![str!("fizz"), str!("buzz")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn append_horizontal_uneven_left() {
            let lhs = mock_block(vec![vec![str!("foo")], vec![str!("fizz")]]);
            let rhs = mock_block(vec![vec![str!("bar")]]);

            assert_eq!(
                lhs.append_horizontal(&rhs),
                Block {
                    width: 7,
                    rows: vec![
                        Row {
                            width: 7,
                            segments: vec![str!("foo"), str!(" "), str!("bar")]
                        },
                        Row {
                            width: 4,
                            segments: vec![str!("fizz")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn append_horizontal_uneven_right() {
            let lhs = mock_block(vec![vec![str!("foo")]]);
            let rhs = mock_block(vec![vec![str!("bar")], vec![str!("fizz")]]);

            assert_eq!(
                lhs.append_horizontal(&rhs),
                Block {
                    width: 7,
                    rows: vec![
                        Row {
                            width: 6,
                            segments: vec![str!("foo"), str!("bar")]
                        },
                        Row {
                            width: 7,
                            segments: vec![str!("   "), str!("fizz")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn normalize() {
            let block = mock_block(vec![vec![str!("a")], vec![str!("foo")], vec![str!("fizz")]]);

            assert_eq!(
                block.normalize(),
                Block {
                    width: 4,
                    rows: vec![
                        Row {
                            width: 4,
                            segments: vec![str!("a"), str!("   ")]
                        },
                        Row {
                            width: 4,
                            segments: vec![str!("foo"), str!(" ")]
                        },
                        Row {
                            width: 4,
                            segments: vec![str!("fizz")]
                        }
                    ]
                }
            );
        }

        #[test]
        fn layout_empty() {
            let blocks = vec![mock_block(vec![])];

            assert_eq!(
                Block::layout(blocks, 1),
                Block {
                    width: 0,
                    rows: vec![]
                }
            );
        }

        #[test]
        fn layout_single() {
            let blocks = vec![mock_block(vec![vec![str!("foo")]])];

            assert_eq!(
                Block::layout(blocks, 1),
                Block {
                    width: 3,
                    rows: vec![Row {
                        width: 3,
                        segments: vec![str!("foo")]
                    }]
                }
            );
        }

        #[test]
        fn layout_multiple() {
            let blocks = vec![
                mock_block(vec![vec![str!("foo")]]),
                mock_block(vec![vec![str!("bar")]]),
                mock_block(vec![vec![str!("fizz")]]),
            ];

            assert_eq!(
                Block::layout(blocks, 1),
                Block {
                    width: 12,
                    rows: vec![Row {
                        width: 12,
                        segments: vec![
                            str!("foo"),
                            str!(" "),
                            str!("bar"),
                            str!(" "),
                            str!("fizz")
                        ]
                    }]
                }
            );
        }
    }
}
