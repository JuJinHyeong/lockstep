import pygame
import socket
import threading

# 게임 화면 설정
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
SCREEN_TITLE = "Player Class with Networking"

# 서버 설정
HOST = '127.0.0.1'
PORT = 9000

class Player:
    """플레이어의 상태 및 행동을 관리하는 클래스"""
    def __init__(self, player_id, x, y, socket):
        self.player_id = player_id
        self.x = x
        self.y = y
        self.socket = socket

    def move(self, dx, dy):
        """플레이어 위치 이동 및 서버에 위치 전송"""
        self.x += dx
        self.y += dy
        self.send_position()

    def send_position(self):
        """플레이어 위치를 서버로 전송"""
        message = f"{self.player_id},{self.x},{self.y}"
        try:
            self.socket.send(message.encode('utf-8'))
        except socket.error:
            print("Failed to send data")

class SimpleGame:
    """게임 전체 로직을 관리하는 클래스"""
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption(SCREEN_TITLE)
        self.clock = pygame.time.Clock()

        self.socket = self.connect_to_server()

        self.player = Player(player_id=id(self), x=100, y=100, socket=self.socket)
        self.players = {}

        self.running = True

        # 데이터 수신 스레드 시작
        self.receive_thread = threading.Thread(target=self.receive_data, daemon=True)
        self.receive_thread.start()

    def connect_to_server(self):
        """서버와의 연결 설정"""
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((HOST, PORT))
            print("Connected to server")
            return client_socket
        except ConnectionRefusedError:
            print("Failed to connect to server")
            self.running = False

    def receive_data(self):
        """서버로부터 데이터를 수신하여 다른 플레이어의 상태를 업데이트"""
        while self.running:
            try:
                data = self.socket.recv(1024).decode('utf-8')
                if data:
                    print(data)
                    player_id, x, y = data.split(',')
                    self.players[player_id] = (float(x), float(y))
            except socket.error:
                break
        print("Stopped receiving data")

    def handle_input(self):
        """키 입력에 따라 플레이어 이동"""
        keys = pygame.key.get_pressed()
        dx = 0
        dy = 0

        if keys[pygame.K_UP]:
            dy = -5
        if keys[pygame.K_DOWN]:
            dy = 5
        if keys[pygame.K_LEFT]:
            dx = -5
        if keys[pygame.K_RIGHT]:
            dx = 5

        if dx or dy:
            self.player.move(dx, dy)

    def draw(self):
        """플레이어와 다른 캐릭터를 화면에 그리기"""
        self.screen.fill((0, 0, 0))  # 배경색 검정

        # 본인 플레이어
        pygame.draw.circle(self.screen, (0, 0, 255), (int(self.player.x), int(self.player.y)), 20)

        # 다른 플레이어들
        for player_id, (x, y) in self.players.items():
            pygame.draw.circle(self.screen, (255, 0, 0), (int(x), int(y)), 20)

        pygame.display.flip()

    def close(self):
        """게임 종료 처리"""
        self.running = False
        try:
            self.socket.shutdown(socket.SHUT_RDWR)
            self.socket.close()
        except socket.error:
            pass
        pygame.quit()

    def run(self):
        """메인 게임 루프"""
        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False

            self.handle_input()
            self.draw()
            self.clock.tick(60)

        self.close()

if __name__ == "__main__":
    game = SimpleGame()
    if game.running:
        game.run()
