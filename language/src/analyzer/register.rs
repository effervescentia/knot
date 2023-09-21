use super::{
    context::{NodeContext, ScopeContext},
    fragment::Fragment,
};

pub trait Identify<R> {
    fn identify(&self, ctx: &ScopeContext) -> R;
}

pub trait ToFragment {
    fn to_fragment<'a>(&'a self) -> Fragment;
}

// TODO: try to remove `Sized`
pub trait Register: Sized
where
    Self::Value<()>: Identify<Self::Value<NodeContext>>,
    Self::Value<NodeContext>: ToFragment,
{
    type Node;
    type Value<C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node;
}
