use super::data::{Output, Strong, Type};
use crate::{
    error::Error,
    infer::{weak, BindingMap, NodeDescriptor},
    Context, Result,
};
use kore::invariant;
use lang::{
    ast,
    types::{self, Kind},
    CanonicalId, Canonicalize, FragmentMap, NodeId,
};
use std::{cell::OnceCell, collections::BTreeMap, rc::Rc};

/// type resolved from the `State` during inference
type ResolvedType<'a> = std::result::Result<types::Type<CanonicalId>, &'a Error>;

type Warning<'a> = (&'a NodeDescriptor, String);

/// partial state for a single round of strong type inference
#[derive(Debug, PartialEq)]
pub struct State<'a> {
    pub context: &'a Context<'a>,

    pub fragments: &'a FragmentMap<NodeId>,

    pub bindings: BindingMap,

    pub nodes: Vec<NodeDescriptor>,

    pub types: BTreeMap<NodeId, Strong>,

    pub warnings: Vec<Warning<'a>>,
}

impl<'a> State<'a> {
    /// create a new `State` from the output of the weak inference phase and the analysis `Context`
    pub fn from_weak(context: &'a Context, mut weak: weak::Output<'a>) -> Self {
        let nodes = weak.build_descriptors(context.namespace_id);

        Self {
            context,
            fragments: weak.fragments,
            bindings: weak.bindings,
            nodes,
            types: Default::default(),
            warnings: Default::default(),
        }
    }

    /// create queue and next `State` from previous
    pub fn next(prev: Self) -> (Vec<NodeDescriptor>, Self) {
        (
            prev.nodes,
            Self {
                nodes: Default::default(),
                ..prev
            },
        )
    }

    /// if true then inference is done
    pub fn is_done(&self) -> bool {
        self.nodes.is_empty()
    }

    /// get the strong type of a node in the target source file
    pub fn get_type(
        &self,
        id: &NodeId,
        allowed_kind: &Kind,
    ) -> Option<&std::result::Result<Type, Error>> {
        self.types.get(id).and_then(|(kind, strong)| {
            if !allowed_kind.can_accept(kind) {
                return None;
            }

            Some(strong)
        })
    }

    pub fn is_local(&self, id: &CanonicalId) -> bool {
        id.0 == self.context.namespace_id
    }

    /// resolve the type of a canonical node ID in the scope of the entire program
    pub fn resolve(&self, id: &CanonicalId, allowed_kind: &Kind) -> Option<ResolvedType> {
        if !self.is_local(id) {
            return self
                .context
                .modules
                .resolve(*id)
                .map(|x| Ok(x.1.to_canonical()));
        }

        self.get_type(&id.1, allowed_kind)
            .and_then(|strong| match strong {
                Ok(Type::Value(local)) => Some(Ok(local.clone())),

                Ok(Type::Inherit(from_id)) => self.resolve_any(from_id),

                Err(err) => Some(Err(err)),
            })
    }

    pub fn resolve_value(&self, id: &CanonicalId) -> Option<ResolvedType> {
        self.resolve(id, &Kind::Value)
    }

    pub fn resolve_any(&self, id: &CanonicalId) -> Option<ResolvedType> {
        self.resolve(id, &Kind::Mixed)
    }

    fn canonicalize_type(
        &self,
        id: NodeId,
        x: &types::Type<CanonicalId>,
        output: &Output,
    ) -> Rc<ast::typed::Meta> {
        Rc::new((
            self.canonicalize(id),
            ast::typed::Type(x.map(&|id| {
                Rc::clone(
                    (if self.is_local(id) {
                        output.types.get(&id.1).and_then(OnceCell::get)
                    } else {
                        self.context.modules.resolve(*id)
                    })
                    .unwrap_or_else(|| invariant!("type not found")),
                )
            })),
        ))
    }

    pub fn into_result(self) -> Result<Output> {
        let output = Output::new(self.types.keys());
        let mut errors = vec![];

        let get_cell = |id| {
            output
                .types
                .get(&id)
                .unwrap_or_else(|| invariant!("no cell exists to store type"))
        };

        for (id, (_, x)) in &self.types {
            match x {
                Ok(Type::Value(x)) => {
                    let cell = get_cell(*id);

                    cell.set(self.canonicalize_type(*id, x, &output)).ok();
                }

                Ok(Type::Inherit(from_id)) => {
                    let cell = get_cell(*id);

                    let value = if self.is_local(from_id) {
                        get_cell(from_id.1)
                            .get()
                            .unwrap_or_else(|| invariant!("inherited cell is empty"))
                    } else {
                        self.context
                            .modules
                            .resolve(*from_id)
                            .unwrap_or_else(|| invariant!("inherited type not found"))
                    };

                    cell.set(Rc::clone(value)).ok();
                }

                Err(err) => errors.push((*id, err.clone())),
            }
        }

        if errors.is_empty() {
            Ok(output)
        } else {
            Err(errors)
        }
    }
}

impl<'a> Canonicalize for State<'a> {
    fn canonicalize(&self, id: NodeId) -> CanonicalId {
        CanonicalId(self.context.namespace_id, id)
    }
}
