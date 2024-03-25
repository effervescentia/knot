mod ast;
mod error;
mod infer;
mod into_typed;

pub use ast::typed;
use error::ResolveError;
use into_typed::Visitor;
use lang::{ModuleReference, NodeId};
use std::collections::HashMap;

pub type Result<Value> = std::result::Result<Value, Vec<(NodeId, ResolveError)>>;

pub struct Context {
    pub namespace: ModuleReference,
    pub modules: HashMap<String, ()>,
}

pub fn analyze<Raw>(ctx: &Context, raw: Raw) -> Result<typed::Program>
where
    Raw: ast::into_fragments::IntoFragments + into_typed::IntoTyped + Clone,
{
    let fragments = raw.clone().into_fragments();

    let weak = infer::weak::infer_types(&fragments);
    let strong = infer::strong::infer_types(ctx, weak)?;

    let typed = raw.into_typed(Visitor::new(strong));

    // do semantic analysis

    Ok(typed)
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        typed::{self, ReferenceType},
        Context,
    };
    use kore::str;
    use lang::{ast::walk, test::mock, types::Type, ModuleReference, ModuleScope, Node};
    use std::collections::HashMap;

    #[test]
    fn empty() {
        let ctx = Context {
            namespace: ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: HashMap::new(),
        };
        let raw = mock::Module(walk::Span::mock(ast::Module::new(vec![], vec![])));

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(typed::Program(typed::Module(Node::mock(
                ast::Module::new(vec![], vec![]),
                ReferenceType(Type::Module(vec![]))
            ))))
        );
    }
}
