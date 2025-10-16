const id = "youtubeFrame";
function init() {
  const tag = document.createElement("script");
  tag.src = "https://www.youtube.com/iframe_api";
  const firstScriptTag = document.getElementsByTagName("script")[0];
  firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);
  console.log("ready");
}

var player;
window.onYouTubeIframeAPIReady = () => {
  console.log("init");
  player = new YT.Player(id, {
    height: "100%",
    width: "100%",
    videoId: "FvHsR0SRTOs",
    playerVars: { autoplay: 1 /*, 'controls': 0 */ },
    events: {
      // onReady: () => {
      // console.log("ready");
      // setTimeout(() => player.playVideo(), 1000);
      // progressInterval = setInterval(onTick, 200);
      // isReady = true;
      // },
      // onStateChange: onPlayerStateChange,
    },
  });

  document.player = player;
  // (document as any).player = player;
};

document.addEventListener("keydown", (e) => {
  if (e.key == "d") init();
  console.log(e);
});
