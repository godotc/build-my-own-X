use std::time::Duration;

use libp2p::core::transport::upgrade;
use libp2p::futures::StreamExt;
use libp2p::noise::{Keypair, X25519};
use libp2p::swarm::SwarmBuilder;
use libp2p::{mplex, Swarm, Transport};
use libp2p::{noise::NoiseConfig, tcp::TokioTcpConfig};
use log::error;
use log::info;
use rs_bc::p2p::{self};
use rs_bc::App;
use tokio::io::AsyncBufReadExt;
use tokio::sync::mpsc;
use tokio::time::sleep;
use tokio::{select, spawn};

#[tokio::main]
async fn main() {
    pretty_env_logger::init();

    info!("Peer Id: {}", p2p::PEER_ID.clone());

    let (responce_sender, mut response_receiver) = mpsc::unbounded_channel::<p2p::ChainResponse>();
    let (init_sender, mut init_receiver) = mpsc::unbounded_channel::<bool>();

    let auth_keys = Keypair::<X25519>::new()
        .into_authentic(&p2p::KEYS)
        .expect("can create auth keys");

    let transport = TokioTcpConfig::new()
        .upgrade(upgrade::Version::V1)
        .authenticate(NoiseConfig::xx(auth_keys).into_authenticated())
        .multiplex(mplex::MplexConfig::new())
        .boxed();

    let behaviour = p2p::AppBehaviour::new(App::new(), responce_sender, init_sender.clone()).await;

    let mut swarm = SwarmBuilder::<p2p::AppBehaviour>::new(transport, behaviour, *p2p::PEER_ID)
        .executor(Box::new(|future| {
            tokio::spawn(future);
        }))
        .build();

    Swarm::listen_on(
        &mut swarm,
        "/ip4/0.0.0.0/tcp/0"
            .parse()
            .expect("can get a local socket"),
    )
    .expect("swarn can be started");

    // here to create the genesis
    spawn(async move {
        sleep(Duration::from_secs(1)).await;
        info!("sending init event");
        init_sender.send(true).expect("can send init event");
    });

    let mut stdin = tokio::io::BufReader::new(tokio::io::stdin()).lines();
    loop {
        let ev = {
            select! {
                line = stdin.next_line()=> {
                    Some(p2p::EventType::Input(line.expect("can get line").expect("can read input")))
                },
                response = response_receiver.recv() =>{
                  Some(p2p::EventType::LocalChainResponse(response.expect("response exists")))
                },
                _init = init_receiver.recv()=>{
                    Some(p2p::EventType::Init)
                },
                event = swarm.select_next_some() =>{
                    info!("Unhandled Swarm Event: {:?}",event);
                    None
                },
            }
        };
        if ev.is_none() {
            continue;
        }
        match ev.unwrap() {
            p2p::EventType::Init => {
                // owning gnesis block
                let peers = p2p::get_list_peers(&swarm);
                info!("connected nodes: {}", peers.len());
                swarm.behaviour_mut().app.genesis();

                if !peers.is_empty() {
                    // request other nodes' blockchain, wait syncing the data
                    let req = p2p::LocalChainRequest {
                        from_peer_id: peers.iter().last().expect("at least one peer").to_string(),
                    };

                    let json = serde_json::to_string(&req).expect("can jsonify request");
                    swarm
                        .behaviour_mut()
                        .floodsub
                        .publish(p2p::CHAIN_TOPIC.clone(), json.as_bytes());
                }
            }
            p2p::EventType::LocalChainResponse(resp) => {
                let json = serde_json::to_string(&resp).expect("can jsonify response");
                swarm
                    .behaviour_mut()
                    .floodsub
                    .publish(p2p::CHAIN_TOPIC.clone(), json.as_bytes());
            }
            p2p::EventType::Input(line) => match line.as_str() {
                "ls p" => p2p::handle_print_peers(&swarm),
                cmd if cmd.starts_with("ls c") => p2p::handle_print_chain(&swarm),
                cmd if cmd.starts_with("create b") => p2p::handle_create_block(cmd, &mut swarm),
                _ => error!("unknown command"),
            },
        }
    }
}
