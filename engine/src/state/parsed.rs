use super::{base::Base, Ast, Module};
use crate::{
    link::ImportGraph,
    report::{Enrich, InternalReport, Report},
    Context, ExecutionError, InternalResult, Link,
};
use kore::Incrementor;
use lang::NamespaceId;
use std::{
    cell::RefCell,
    ops::{Deref, DerefMut},
    rc::Rc,
};

#[derive(Clone, Default)]
pub struct Parsed(pub Base<()>, pub Rc<RefCell<Incrementor>>);

impl Parsed {
    pub fn incrementor(&self) -> Rc<RefCell<Incrementor>> {
        Rc::clone(&self.1)
    }

    pub fn to_import_graph(&self) -> ImportGraph {
        self.internal_modules()
            .fold(ImportGraph::new(), |mut graph, (_, x)| {
                graph.add_node(x.id);
                graph
            })
    }

    pub fn link_modules<R>(&self, context: &mut Context<R>) -> InternalResult<ImportGraph> {
        self.internal_modules()
            .try_fold(self.to_import_graph(), |mut acc, (link, module)| {
                let links = module.ast.to_links(link);

                for x in &links {
                    if let Some(x) = self.get_id_by_link(x) {
                        acc.add_edge(&module.id, x).ok();
                    } else {
                        context.raise(ExecutionError::UnregisteredModule(x.clone()))?;
                    }
                }

                Ok(acc)
            })
    }

    pub fn register_source(&mut self, link: Link, text: String, ast: Ast<()>) {
        let incrementor = self.incrementor();
        let module = Module {
            id: NamespaceId(incrementor.borrow_mut().deref_mut().increment()),
            text,
            ast,
        };

        self.0.register_module(link, module)
    }
}

impl Deref for Parsed {
    type Target = Base<()>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Parsed {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl Enrich for Parsed {
    fn enrich(&self, internal: InternalReport) -> Report {
        self.0.enrich(internal)
    }
}
