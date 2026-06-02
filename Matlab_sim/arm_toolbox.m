%% ============================================================
% 柔性机械臂 MDH — Peter Corke Robotics Toolbox (RTB) 版本
% 结构: 云台(2R) + 钢带伸缩(1P) + 连杆x3(3R) = 6-DOF
%
% 新增: 末端画圆轨迹 + 加密工作空间 + 远端稀疏采样
% ============================================================
clear; clc; close all;

if ~exist('Link', 'class')
    error('未检测到 RTB。请先安装并运行 startup_rvc。');
end

%% ===== 1. 几何参数 =====
L0     = 0.15;
L1     = 0.10;
L4     = 0.10;   % 连杆1 (绕伸缩方向)
L5     = 0.30;   % 连杆2 (肘部)
L6     = 0.25;   % 连杆3 (末端)
d_plat = 0.05;

%% ===== 2. 构建机器人 =====
L(1) = Link([0,  L0,     0,  0],     'modified');
L(2) = Link([0,  L1,     0, -pi/2],  'modified');
L(3) = Link([0,  0.40,   0,  pi/2, 1], 'modified');  % P
L(4) = Link([0,  d_plat, 0,  0],     'modified');
L(5) = Link([0,  0,      L4, -pi/2], 'modified');
L(6) = Link([0,  L6,     L5, 0],     'modified');

L(1).qlim = [-pi,   pi];
L(2).qlim = [-pi/3, pi/3];
L(3).qlim = [0.20,  0.80];
L(4).qlim = [-pi,   pi];
L(5).qlim = [-pi/2, pi/2];
L(6).qlim = [-pi/2, pi/2];

robot = SerialLink(L, 'name', '柔性机械臂 (6-DOF)');

fprintf('============================================\n');
fprintf('柔性机械臂 6-DOF (R-R-P-R-R-R)\n');
fprintf('============================================\n');
robot.display();

%% ===== 3. 正向运动学 =====
fprintf('\n========== 正向运动学 ==========\n');
q_test = [0, pi/6, 0.50, pi/3, -pi/4, pi/6];
T_end  = robot.fkine(q_test);
fprintf('末端位置: [%.4f, %.4f, %.4f] m\n', double(transl(T_end)));

%% ===== 4. 可视化 =====
fprintf('\n========== 构型图 ==========\n');
figure(1);
robot.plot(q_test, 'workspace', [-1.5 1.5 -1.5 1.5 0 2], 'scale', 0.5);
title(sprintf('构型: [%.0f°,%.0f°,%.2fm,%.0f°,%.0f°,%.0f°]', rad2deg(q_test)));

%% ===== 5. 工作空间 (加密 + 远端稀疏) =====
fprintf('\n========== 工作空间 (加密, 远端稀疏) ==========\n');

N_ws = 80000;  % 增大采样量
pts  = zeros(N_ws, 3);

lims = [-pi,pi; -pi/3,pi/3; 0.20,0.80; -pi,pi; -pi/2,pi/2; -pi/2,pi/2];

fprintf('采样中 (%d 点)...', N_ws);

for k = 1:N_ws
    % 关节1,2,4,5,6: 均匀采样
    qk = zeros(1,6);
    qk(1) = lims(1,1) + rand * (lims(1,2) - lims(1,1));
    qk(2) = lims(2,1) + rand * (lims(2,2) - lims(2,1));

    % ---- 关键: d3 非均匀采样 (远端稀疏) ----
    % 用平方根映射: d3 = d_min + (d_max - d_min) * sqrt(rand)
    % sqrt(rand) 使小值出现概率更高 → 近端点密集, 远端点稀疏
    d3_min = lims(3,1);
    d3_max = lims(3,2);
    qk(3) = d3_min + (d3_max - d3_min) * sqrt(rand);

    qk(4) = lims(4,1) + rand * (lims(4,2) - lims(4,1));
    qk(5) = lims(5,1) + rand * (lims(5,2) - lims(5,1));
    qk(6) = lims(6,1) + rand * (lims(6,2) - lims(6,1));

    Tk = robot.fkine(qk);
    pts(k,:) = double(transl(Tk));
end
fprintf(' 完成\n');

% 计算每个点到原点的距离, 用于 color mapping
dist = sqrt(pts(:,1).^2 + pts(:,2).^2 + pts(:,3).^2);

figure('Name', '工作空间 (加密+远端稀疏)', 'NumberTitle', 'off', ...
       'Position', [100, 80, 1000, 750]);
scatter3(pts(:,1), pts(:,2), pts(:,3), 2, dist, ...
         'filled', 'MarkerFaceAlpha', 0.08);
colormap(flipud(jet));
cb = colorbar;
cb.Label.String = '距原点距离 (m)';
xlabel('X (m)'); ylabel('Y (m)'); zlabel('Z (m)');
title(sprintf('工作空间 (N=%d, d_3~sqrt(rand) 远端稀疏)', N_ws));
axis equal; grid on; view(35, 25);

%% ===== 6. 末端画圆轨迹 =====
fprintf('\n========== 末端画圆轨迹 ==========\n');

% 定义空间圆参数
circle_center = [0.5; 0; 0.8];   % 圆心 (在机械臂可达范围内)
circle_radius = 0.15;             % 半径
circle_normal = [0; 0; 1];        % 圆平面法向 (水平面)
N_circle = 200;                   % 轨迹点数

% 生成圆的位姿序列
T_circle_seq = repmat(SE3, 1, N_circle);
for k = 1:N_circle
    angle = 2 * pi * (k-1) / N_circle;

    % 圆上点的位置
    px = circle_center(1) + circle_radius * cos(angle);
    py = circle_center(2) + circle_radius * sin(angle);
    pz = circle_center(3);

    % 姿态: 末端Z轴始终垂直向下 (或根据需要调整)
    % 这里让末端保持固定朝向
    T_circle_seq(k) = SE3(rotz(angle) * roty(0), [px; py; pz]);
end

% 用 IK 求解每个轨迹点对应的关节角
fprintf('IK 求解 %d 个轨迹点...\n', N_circle);
q_circle = zeros(N_circle, 6);

% 第一帧: 从初始猜测开始
q_init = [0, 0, 0.4, 0, 0, 0];
ik_solver = robot.ikine(T_circle_seq(1), 'q0', q_init, ...
                        'ilimit', 500, 'tol', 1e-6);
q_circle(1,:) = ik_solver;

% 后续帧: 用前一帧的结果作为当前帧的初始值 (warm start)
for k = 2:N_circle
    try
        qk = robot.ikine(T_circle_seq(k), 'q0', q_circle(k-1,:), ...
                         'ilimit', 200, 'tol', 1e-6);
        q_circle(k,:) = qk;
    catch
        % ikine 失败则用 ikcon
        q_circle(k,:) = robot.ikcon(T_circle_seq(k), q_circle(k-1,:));
    end
end

fprintf('IK 求解完成。\n');

% 绘制圆轨迹的关节角度
figure('Name', '画圆 — 关节角度', 'NumberTitle', 'off', ...
       'Position', [50, 80, 1000, 700]);
t_c = linspace(0, 2*pi, N_circle);
jnames = {'Pan (°)','Tilt (°)','伸缩 (m)','连杆1 (°)','连杆2 (°)','连杆3 (°)'};
q_deg_c = [rad2deg(q_circle(:,[1,2,4,5,6]))];
q_plot_c = [q_deg_c(:,1:2), q_circle(:,3), q_deg_c(:,3:5)];

for j = 1:6
    subplot(6,1,j);
    plot(t_c/pi, q_plot_c(:,j), 'b-', 'LineWidth', 1.0);
    xlim([0, 2]);  % 0 到 2*pi/pi = 2
    ylabel(jnames{j});
    if j == 1, title('画圆过程中各关节角度 (x轴: 圆角度/π)'); end
    if j == 6, xlabel('圆角度 / π'); end
    grid on;
end

% 画圆动画
fprintf('\n播放画圆动画...\n');

figure('Name', '末端画圆轨迹', 'NumberTitle', 'off', ...
       'Position', [150, 80, 1000, 750]);

% 预计算末端实际位置用于画轨迹线
ee_actual = zeros(3, N_circle);
for k = 1:N_circle
    Tk = robot.fkine(q_circle(k,:));
    ee_actual(:,k) = double(transl(Tk));
end

fps = 30;
dt  = 1 / fps;

for k = 1:N_circle
    robot.plot(q_circle(k,:), 'workspace', [-1.5 1.5 -1.5 1.5 0 2], ...
               'scale', 0.5, 'delay', 0, 'nowrist');
    hold on;

    % 画目标圆 (虚线)
    target_circle = zeros(3, N_circle);
    for kk = 1:N_circle
        target_circle(:,kk) = double(transl(T_circle_seq(kk)));
    end
    plot3(target_circle(1,:), target_circle(2,:), target_circle(3,:), ...
          'm--', 'LineWidth', 1.5);

    % 画末端已走过的轨迹
    plot3(ee_actual(1,1:k), ee_actual(2,1:k), ee_actual(3,1:k), ...
          'g-', 'LineWidth', 1.5);

    % 当前点高亮
    plot3(ee_actual(1,k), ee_actual(2,k), ee_actual(3,k), ...
          'go', 'MarkerSize', 10, 'MarkerFaceColor', 'g');

    hold off;
    title(sprintf('画圆: %.0f° / 360°  |  位置误差: %.3f mm', ...
          rad2deg(2*pi*(k-1)/N_circle), ...
          norm(ee_actual(:,k) - target_circle(:,k)) * 1000));
    drawnow;
    pause(dt);
end

% 画圆精度评估
circle_err = zeros(1, N_circle);
for k = 1:N_circle
    circle_err(k) = norm(ee_actual(:,k) - target_circle(:,k));
end
fprintf('画圆完成。\n');
fprintf('  最大位置误差: %.3f mm\n', max(circle_err) * 1000);
fprintf('  平均位置误差: %.3f mm\n', mean(circle_err) * 1000);

%% ===== 7. 直线轨迹 (补充示例) =====
fprintf('\n========== 直线轨迹 ==========\n');

p_start = [0.30; 0.20; 0.60];
p_end   = [0.70; 0.00; 1.00];
N_line  = 80;

% ctraj: 笛卡尔空间直线轨迹
T_line_seq = ctraj(SE3(p_start), SE3(p_end), N_line);

fprintf('IK 求解 %d 个直线轨迹点...\n', N_line);
q_line = zeros(N_line, 6);
q_init_line = q_circle(end,:);  % 用画圆末帧做初值

q_line(1,:) = robot.ikine(T_line_seq(1), 'q0', q_init_line, ...
                           'ilimit', 500, 'tol', 1e-6);
for k = 2:N_line
    try
        q_line(k,:) = robot.ikine(T_line_seq(k), 'q0', q_line(k-1,:), ...
                                  'ilimit', 200, 'tol', 1e-6);
    catch
        q_line(k,:) = robot.ikcon(T_line_seq(k), q_line(k-1,:));
    end
end
fprintf('直线轨迹 IK 完成。\n');

%% ===== 8. 动力学 =====
fprintf('\n========== 动力学 ==========\n');

robot.links(1).m = 2.0;   robot.links(1).r = [0; 0; L0/2];
robot.links(2).m = 1.0;   robot.links(2).r = [0; 0; L1/2];
robot.links(3).m = 0.3;   robot.links(3).r = [0; 0; 0.2];
robot.links(4).m = 0.5;   robot.links(4).r = [0; 0; d_plat/2];
robot.links(5).m = 1.2;   robot.links(5).r = [L4/2; 0; 0];
robot.links(6).m = 0.8;   robot.links(6).r = [L5/2; 0; L6/2];

robot.gravity = [0; 0; -9.81];

q_dyn = q_test;
tau   = robot.rne(q_dyn, zeros(1,6), zeros(1,6));
fprintf('重力补偿力矩 (Nm): [%.2f, %.2f, %.2f, %.2f, %.2f, %.2f]\n', tau);

%% ===== 9. teach =====
fprintf('\n========================================\n');
fprintf('自动示例全部完成。\n');
fprintf('即将打开 teach 窗口...\n');
fprintf('========================================\n');

figure('Name', '交互式示教', 'NumberTitle', 'off', ...
       'Position', [100, 80, 900, 700]);
robot.teach(q_test);
fprintf('结束。\n');
