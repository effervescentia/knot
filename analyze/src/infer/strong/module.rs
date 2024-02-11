use crate::context::StrongContext;
use lang::{
    ast::{self, explode, walk},
    types,
};

use super::Strong;

pub fn infer(declarations: &[walk::NodeId], ctx: &StrongContext) -> Option<Strong> {
    let typed_declarations = declarations
        .iter()
        .map(|x| match ctx.fragments.0.get(x)? {
            (
                _,
                explode::Fragment::Declaration(
                    ast::Declaration::TypeAlias { storage, .. }
                    | ast::Declaration::Enumerated { storage, .. }
                    | ast::Declaration::Constant { storage, .. }
                    | ast::Declaration::Function { storage, .. }
                    | ast::Declaration::View { storage, .. }
                    | ast::Declaration::Module { storage, .. },
                ),
            ) => {
                let (kind, _) = ctx.refs.get(x)?;

                Some((storage.binding, *kind, *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>()?;

    Some(Ok(types::Type::Module(typed_declarations)))
}

#[cfg(test)]
mod tests {
    use crate::{context::StrongContext, infer::strong::Strong, test::fixture::strong_ctx_from};
    use kore::str;
    use lang::{
        ast::{
            self,
            explode::{self, ScopeId},
            walk::NodeId,
        },
        types,
    };

    fn infer(declarations: &[NodeId], ctx: &StrongContext) -> Option<Strong> {
        super::infer(declarations, ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(infer(&[NodeId(0)], &ctx), None);
    }

    #[test]
    fn declarations() {
        let ctx = strong_ctx_from(
            vec![
                (
                    NodeId(0),
                    (
                        ScopeId(vec![0]),
                        explode::Fragment::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::public(str!("foo")),
                            value_type: None,
                            value: NodeId(0),
                        }),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        ScopeId(vec![0]),
                        explode::Fragment::Declaration(ast::Declaration::TypeAlias {
                            storage: ast::Storage::public(str!("bar")),
                            value: NodeId(2),
                        }),
                    ),
                ),
            ],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            infer(&[NodeId(0), NodeId(1)], &ctx),
            Some(Ok(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, 0),
                (str!("bar"), types::RefKind::Type, 1)
            ])))
        );
    }

    #[test]
    #[ignore = "not finished"]
    fn imports() {
        let ctx = strong_ctx_from(
            vec![
                (
                    NodeId(0),
                    (
                        ScopeId(vec![0]),
                        explode::Fragment::Import(ast::Import {
                            source: ast::ImportSource::Local,
                            path: vec![],
                            alias: None,
                        }),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        ScopeId(vec![0]),
                        explode::Fragment::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::public(str!("foo")),
                            value_type: None,
                            value: NodeId(2),
                        }),
                    ),
                ),
            ],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            infer(&[NodeId(0), NodeId(1)], &ctx),
            Some(Ok(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, 0),
                (str!("bar"), types::RefKind::Type, 1)
            ])))
        );
    }
}
