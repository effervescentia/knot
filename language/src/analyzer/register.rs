use super::{
    context::{NodeContext, ScopeContext},
    fragment::Fragment,
};

pub trait Identify<R> {
    fn identify(self, ctx: &mut ScopeContext) -> R;
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

    fn register(self, ctx: &mut ScopeContext) -> Self::Node;

    // fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext>;

    // fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Self::Ref;
}
