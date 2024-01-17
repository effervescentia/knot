use super::traversal::NodeId;

#[derive(Debug, PartialEq)]
pub enum Fragment {
    Expression(super::Expression<NodeId, NodeId, NodeId>),
    Statement(super::Statement<NodeId>),
    Component(super::Component<NodeId, NodeId>),
    Parameter(super::Parameter<NodeId, NodeId>),
    Declaration(super::Declaration<NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(super::TypeExpression<NodeId>),
    Import(super::Import),
    Module(super::Module<NodeId, NodeId>),
}

#[derive(Default)]
pub struct Context {
    next_id: usize,
    fragments: Vec<(NodeId, Fragment)>,
}

impl Context {
    pub fn register(mut self, fragment: Fragment) -> (NodeId, Self) {
        let id = NodeId(self.next_id);
        self.next_id += 1;
        self.fragments.push((id, fragment));
        (id, self)
    }

    pub fn into_fragments(self) -> Vec<(NodeId, Fragment)> {
        self.fragments
    }
}

pub trait Capture<T>: Sized {
    fn capture<F>(self, f: F, context: Context) -> Vec<(NodeId, Fragment)>
    where
        F: Fn(T, Context) -> (NodeId, Context);
}
