%% ============================================================
% 柔性机械臂 MDH 运动学 — 手写版本
% 结构: 云台(2-DOF) + 钢带伸缩(1-DOF) + 连杆×3(3-DOF) = 6-DOF
%        R  +  R  +   P   +   R    +   R    +   R
% -----------------------------------------------------------
% 连杆1: 绕伸缩方向旋转 (关节4)
% 连杆2: 与连杆1正交, 肘部 (关节5)
% 连杆3: 末端 (关节6)
% ============================================================
clear; clc; close all;

fprintf('============================================\n');
fprintf('柔性机械臂 MDH 手写运动学\n');
fprintf('关节: R-R-P-R-R-R (6-DOF)\n');
fprintf('============================================\n\n');

%% ===== 几何参数 =====
L0     = 0.15;   % 底座高度
L1     = 0.10;   % tilt 偏距
L4     = 0.10;   % 连杆1长度 (绕伸缩方向旋转)
L5     = 0.30;   % 连杆2长度 (肘部)
L6     = 0.25;   % 连杆3长度 (末端)
d_plat = 0.05;   % 伸缩末端平台偏距

params = [L0, L1, L4, L5, L6, d_plat];

%% ===== MDH 参数表 =====
%   i | α_{i-1} | a_{i-1} | θ_i       | d_i      | 关节
%  ---|----------|----------|-----------|----------|------
%   1 |    0     |    0     | θ₁*       | L0       | R  云台Pan
%   2 |  -π/2    |    0     | θ₂*       | L1       | R  云台Tilt
%   3 |   π/2    |    0     | 0         | d₃*      | P  钢带伸缩
%   4 |    0     |    0     | θ₄*       | d_plat   | R  连杆1(绕伸缩)
%   5 |  -π/2    |   L4     | θ₅*       | 0        | R  连杆2(肘部)
%   6 |    0     |   L5     | θ₆*       | L6       | R  连杆3(末端)

%% ===== 正向运动学 =====
fprintf('\n>>> 示例1: 正向运动学\n');

q_test = [0, pi/6, 0.50, pi/3, -pi/4, pi/6];

[T_all, T_chain] = fkine_mdh(q_test, params);
T_end = T_all(:,:,end);

fprintf('关节: [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_test));
fprintf('末端: [%.4f, %.4f, %.4f] m\n', T_end(1,4), T_end(2,4), T_end(3,4));

% 打印 MDH 表
fprintf('\n--- MDH 参数表 (当前构型) ---\n');
fprintf(' i | α_{i-1}(°) | a_{i-1}(m) | θ_i(°)  | d_i(m)   | 关节\n');
fprintf('---|-------------|-------------|----------|----------|----------\n');
labels  = {'云台Pan','云台Tilt','钢带伸缩','连杆1(绕伸缩)','连杆2(肘)','连杆3(末端)'};
alphas  = [0, -90, 90, 0, -90, 0];
a_vals  = [0, 0, 0, 0, L4, L5];
th_deg  = rad2deg([q_test(1), q_test(2), 0, q_test(4), q_test(5), q_test(6)]);
d_vals  = [L0, L1, q_test(3), d_plat, 0, L6];
for i = 1:6
    fprintf(' %d | %11.0f | %11.3f | %8.1f | %8.3f | %s\n', ...
            i, alphas(i), a_vals(i), th_deg(i), d_vals(i), labels{i});
end

%% ===== 雅可比 =====
fprintf('\n>>> 示例2: 几何雅可比 (6×6)\n');
J = jacobian_geom(q_test, params);
fprintf('线速度部分 (前三行):\n');  disp(J(1:3,:));
fprintf('角速度部分 (后三行):\n');  disp(J(4:6,:));

%% ===== 可视化 =====
fprintf('\n>>> 示例3: 三维构型\n');
figure('Name', '机械臂构型', 'NumberTitle', 'off', ...
       'Position', [50, 80, 900, 700]);
plot_arm(q_test, params, gcf);

%% ===== 工作空间 =====
fprintf('\n>>> 示例4: 工作空间\n');
workspace_analysis(params);

%% ===== 逆运动学 =====
fprintf('\n>>> 示例5: 逆运动学验证\n');

q_target = [pi/4; pi/4; 0.5; pi/3; -pi/3; pi/6];
fprintf('目标: [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_target));

T_target = fkine_mdh(q_target, params);
T_tar = T_target(:,:,end);

q_init = ikine_analytic(T_tar, params);
q_ik   = ikine_numeric(T_tar, q_init, params);

fprintf('解析初值:  [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_init));
fprintf('数值精化:  [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_ik));

T_verify = fkine_mdh(q_ik, params);
err = norm(T_tar(1:3,4) - T_verify(1:3,4,end));
fprintf('位置误差: %.4e m\n', err);

%% ===== 轨迹规划 =====
fprintf('\n>>> 示例6: 轨迹规划\n');

q_start = [0;   0;    0.25; 0;     0;     0];
q_end   = [pi/3; pi/8; 0.60; pi/2; -pi/4; pi/6];
N_pts   = 100;
T_dur   = 3;

fprintf('起点: [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_start));
fprintf('终点: [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_end));

q_traj = trajectory_plan(q_start, q_end, N_pts, T_dur);
fprintf('开始动画...\n');
animate_trajectory(q_traj, params, 25);

fprintf('\n========================================\n');
fprintf('全部示例运行完成。\n');
fprintf('========================================\n');


%% ============================================================
%  局部函数定义 (MATLAB 要求位于脚本末尾)
% ============================================================

function T = dh_transform(alpha, a, theta, d)
    % MDH 单步变换 (Craig): T = Rx(alpha)*Dx(a)*Rz(theta)*Dz(d)
    T = [cos(theta),            -sin(theta),           0,           a;
         sin(theta)*cos(alpha), cos(theta)*cos(alpha), -sin(alpha), -sin(alpha)*d;
         sin(theta)*sin(alpha), cos(theta)*sin(alpha), cos(alpha),  cos(alpha)*d;
         0,                     0,                     0,           1];
end

function [T_all, T_chain] = fkine_mdh(q, params)
    q = q(:);
    L0 = params(1); L1 = params(2); L4 = params(3);
    L5 = params(4); L6 = params(5); d_plat = params(6);

    dh_table = [
         0,      0,    q(1),  L0;       % 1: Pan
        -pi/2,   0,    q(2),  L1;       % 2: Tilt
         pi/2,   0,    0,     q(3);     % 3: 伸缩 (P)
         0,      0,    q(4),  d_plat;   % 4: 连杆1 (绕伸缩方向)
        -pi/2,   L4,   q(5),  0;        % 5: 连杆2 (肘部)
         0,      L5,   q(6),  L6;       % 6: 连杆3 (末端)
    ];

    T = eye(4);
    T_chain = zeros(4,4,7);
    T_chain(:,:,1) = T;

    for i = 1:6
        alpha = dh_table(i,1); a = dh_table(i,2);
        theta = dh_table(i,3); d = dh_table(i,4);
        T = T * dh_transform(alpha, a, theta, d);
        T_chain(:,:,i+1) = T;
    end
    T_all = T_chain(:,:,2:7);
end

function J = jacobian_geom(q, params)
    [~, T_chain] = fkine_mdh(q, params);
    o_end = T_chain(1:3, 4, end);
    J = zeros(6, 6);

    for i = 1:6
        T_i = T_chain(:,:,i);
        z_i = T_i(1:3, 3);
        o_i = T_i(1:3, 4);

        if i == 3  % 移动副
            J(1:3, i) = z_i;
            J(4:6, i) = [0; 0; 0];
        else       % 旋转副
            J(1:3, i) = cross(z_i, o_end - o_i);
            J(4:6, i) = z_i;
        end
    end
end

function workspace_analysis(params)
    N = 20000;
    lims = [-pi,pi; -pi/3,pi/3; 0.20,0.80; -pi,pi; -pi/2,pi/2; -pi/2,pi/2];
    pts = zeros(N, 3);
    for k = 1:N
        qk = lims(:,1) + rand(6,1) .* (lims(:,2) - lims(:,1));
        Tk = fkine_mdh(qk, params);
        pts(k,:) = Tk(1:3, 4, end)';
    end
    figure('Name', '工作空间', 'NumberTitle', 'off', ...
           'Position', [100, 80, 900, 700]);
    scatter3(pts(:,1), pts(:,2), pts(:,3), 4, pts(:,3), ...
             'filled', 'MarkerFaceAlpha', 0.12);
    colormap(jet); colorbar;
    xlabel('X'); ylabel('Y'); zlabel('Z');
    title(sprintf('工作空间 (N=%d)', N));
    axis equal; grid on; view(30,30);
    fprintf('  采样点数: %d\n', N);
end

function plot_arm(q, params, fig)
    [~, T_chain] = fkine_mdh(q, params);
    n = size(T_chain, 3);
    pos = zeros(3, n);
    for i = 1:n
        pos(:,i) = T_chain(1:3, 4, i);
    end

    figure(fig); clf; hold on;

    % 连杆线
    plot3(pos(1,:), pos(2,:), pos(3,:), 'b-o', ...
          'LineWidth', 2.5, 'MarkerSize', 8, 'MarkerFaceColor', 'r');

    % 关节球
    [xs, ys, zs] = sphere(20);
    for i = 1:n
        surf(xs*0.025+pos(1,i), ys*0.025+pos(2,i), ...
             zs*0.025+pos(3,i), 'FaceColor', [0.3,0.5,0.8], ...
             'EdgeColor', 'none', 'FaceAlpha', 0.7);
    end

    % 基座
    [cx, cy, cz] = cylinder(0.06, 20);
    surf(cx, cy, cz*0.03, 'FaceColor', [0.4,0.4,0.4], 'EdgeColor', 'none');

    % 末端坐标系
    T_end = T_chain(:,:,end);
    al = 0.08;
    quiver3(T_end(1,4), T_end(2,4), T_end(3,4), ...
            T_end(1,1)*al, T_end(2,1)*al, T_end(3,1)*al, 'r', 'LineWidth', 1.5);
    quiver3(T_end(1,4), T_end(2,4), T_end(3,4), ...
            T_end(1,2)*al, T_end(2,2)*al, T_end(3,2)*al, 'g', 'LineWidth', 1.5);
    quiver3(T_end(1,4), T_end(2,4), T_end(3,4), ...
            T_end(1,3)*al, T_end(2,3)*al, T_end(3,3)*al, 'b', 'LineWidth', 1.5);

    % 末端标记
    plot3(pos(1,end), pos(2,end), pos(3,end), 'g^', ...
          'MarkerSize', 15, 'MarkerFaceColor', 'g');

    % 标注
    text(0.09, 0, 0,        '基座',   'FontSize', 9);
    text(pos(1,2), pos(2,2), pos(3,2)+0.04, '云台', 'FontSize', 9);
    text(pos(1,3), pos(2,3), pos(3,3)+0.04, '伸缩', 'FontSize', 9);
    text(pos(1,end), pos(2,end), pos(3,end)+0.04, '末端', 'FontSize', 9);

    xlabel('X'); ylabel('Y'); zlabel('Z');
    title(sprintf('构型: q=[%.0f°,%.0f°,%.2fm,%.0f°,%.0f°,%.0f°]', rad2deg(q)));
    axis equal; grid on;
    xlim([-1.5 1.5]); ylim([-1.5 1.5]); zlim([0 2]);
    view(45, 25);
    hold off;
end

function q0 = ikine_analytic(T_target, params)
    L0 = params(1); L1 = params(2); L5 = params(4); L6 = params(5);

    p = T_target(1:3, 4);
    R_target = T_target(1:3, 1:3);

    % 步骤1: theta1 (Pan) — X-Y 投影
    theta1 = atan2(p(2), p(1));

    % 步骤2: 腕部 = 末端向后退 (L5+L6 近似)
    z_end = R_target(:, 3);
    w = p - (L5 + L6) * z_end;
    wx = w(1); wy = w(2); wz = w(3);

    % 步骤3: 几何法求 theta2 和 d3
    h_wrist = wz - L0;
    r_wrist = sqrt(wx^2 + wy^2);
    eq_val  = max(0, r_wrist^2 + h_wrist^2 - L1^2);
    d3_est  = sqrt(eq_val);
    theta2_est = atan2(h_wrist, r_wrist) - atan2(L1, d3_est);

    % 步骤4: 初猜后3关节
    q_temp = [theta1; theta2_est; d3_est; 0; 0; 0];
    [~, T_c] = fkine_mdh(q_temp, params);
    T_03 = T_c(:,:,4);
    R_03 = T_03(1:3, 1:3);
    R_36 = R_03' * R_target;

    % R_36 = Rz(θ4)*Rz(θ5)*Rz(θ6) 简化 (三轴平行时有无穷解, 取一种分配)
    theta6 = atan2(R_36(2,1), R_36(1,1)) / 3;
    theta5 = theta6;
    theta4 = theta6;

    q0 = [theta1; theta2_est; d3_est; theta4; theta5; theta6];
end

function q = ikine_numeric(T_target, q0, params, max_iter, tol)
    if nargin < 5, max_iter = 200; end
    if nargin < 6, tol = 1e-6; end

    lambda = 0.1;
    q = q0(:);

    for iter = 1:max_iter
        T_cur = fkine_mdh(q, params);
        T_cur = T_cur(:,:,end);

        dp = T_target(1:3, 4) - T_cur(1:3, 4);
        dR = T_target(1:3, 1:3) * T_cur(1:3, 1:3)';
        dtheta_vec = vex(logm_safe(dR));
        e = [dp; dtheta_vec];

        if norm(e) < tol, break; end

        J = jacobian_geom(q, params);
        dq = (J'*J + lambda*eye(6)) \ (J' * e);
        q = q + dq;
    end
end

function R = logm_safe(M)
    theta = acos(max(-1, min(1, (trace(M) - 1) / 2)));
    if abs(theta) < 1e-10
        R = zeros(3,3);
    else
        R = (theta / (2*sin(theta))) * (M - M');
    end
end

function v = vex(S)
    v = [S(3,2); S(1,3); S(2,1)];
end

function q_traj = trajectory_plan(q_start, q_end, N, duration)
    t = linspace(0, duration, N);
    q_traj = zeros(6, N);
    for j = 1:6
        s = quintic_smooth(0, 1, t);
        q_traj(j,:) = q_start(j) + (q_end(j) - q_start(j)) * s;
    end
end

function s = quintic_smooth(q0, q1, t)
    T = t(end);
    tau = t / T;
    s = 10*tau.^3 - 15*tau.^4 + 6*tau.^5;
end

function animate_trajectory(q_traj, params, fps)
    if nargin < 3, fps = 20; end
    N = size(q_traj, 2);
    dt = 1 / fps;

    fig = figure('Name', '轨迹动画', 'NumberTitle', 'off', ...
                 'Position', [150, 80, 1000, 750]);
    traj_pts = zeros(3, N);
    for k = 1:N
        Tk = fkine_mdh(q_traj(:,k), params);
        traj_pts(:,k) = Tk(1:3, 4, end);
    end

    for k = 1:N
        plot_arm(q_traj(:,k), params, fig);
        hold on;
        plot3(traj_pts(1,1:k), traj_pts(2,1:k), traj_pts(3,1:k), ...
              'g--', 'LineWidth', 1.0);
        hold off;
        drawnow;
        pause(dt);
    end
end
